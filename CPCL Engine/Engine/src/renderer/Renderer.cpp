#include "renderer/Renderer.h"
#include "Vec4_SSE.h"



static uint32_t lerp_color(uint32_t a, uint32_t b, float t)
{
    auto ch = [&](int s) {
        float ca = (float)((a >> s) & 0xFF), cb = (float)((b >> s) & 0xFF);
        return (uint32_t)(ca + (cb - ca) * t);
        };
    return (ch(16) << 16) | (ch(8) << 8) | ch(0);
}


void Renderer::begin_frame(uint32_t clear_color)
{
    _fb.Clear(clear_color);
    _zb.clear_Zbuffer();           // profondeur remise à « infini »
    _tris.clear();
    for (auto& b : _bins) b.clear();
}

void Renderer::end_Frame()
{
    const int W = _fb.getWidth(), H = _fb.getHeight();

    // 1) binning : à quelles tuiles appartient chaque triangle ?
    for (int i = 0; i < (int)_tris.size(); i++) {
        const ScreenTri& s = _tris[i];
        int minx = std::max(0, std::min({ s.x[0], s.x[1], s.x[2] }));
        int maxx = std::min(W - 1, std::max({ s.x[0], s.x[1], s.x[2] }));
        int miny = std::max(0, std::min({ s.y[0], s.y[1], s.y[2] }));
        int maxy = std::min(H - 1, std::max({ s.y[0], s.y[1], s.y[2] }));
        if (minx > maxx || miny > maxy) continue;

        for (int ty = miny / TILE; ty <= maxy / TILE; ty++)
            for (int tx = minx / TILE; tx <= maxx / TILE; tx++)
                _bins[ty * _tilesX + tx].push_back(i);
    }

    std::atomic<int> next{ 0 };
    const int nTiles = _tilesX * _tilesY;

    auto worker = [&]() {
        for (;;) {
            int t = next.fetch_add(1);
            if (t >= nTiles) break;

            int tx = t % _tilesX, ty = t / _tilesX;
            TileRect r{ tx * TILE, ty * TILE,
                        std::min((tx + 1) * TILE, W),
                        std::min((ty + 1) * TILE, H)
            };

            for (int i : _bins[t]) {
                const ScreenTri& s = _tris[i];
                _rtz.draw_triangle_phong(
                    s.x[0], s.y[0], s.z[0], s.iw[0], s.uv[0], s.n[0],
                    s.x[1], s.y[1], s.z[1], s.iw[1], s.uv[1], s.n[1],
                    s.x[2], s.y[2], s.z[2], s.iw[2], s.uv[2], s.n[2],
                    *s.mat, s.L, s.H, r, true);
            }
        }
        };

    const int nThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    for (int i = 1; i < nThreads; i++) pool.emplace_back(worker);
    worker();                                  // le thread principal travaille aussi
    for (auto& th : pool) th.join();           // on attend que tout soit fini
}



void Renderer::draw_particles(const ParticleSystem& ps, const Maths::Matrix4x4_SSE& vp, float focal_px)
{
    const int W = _fb.getWidth(), H = _fb.getHeight();
    Maths::Vec4_SSE tmp;

    for (const Particle& p : ps.particles) {
        Maths::Vec4_SSE clip = vp * tmp.FromPoint(p.pos);
        const float w = clip.reg.m128_f32[3];
        if (w < 0.1f) continue;                              // derrière / trop près de la caméra

        const float inv = 1.0f / w;
        const int   sx = (int)((clip.reg.m128_f32[0] * inv + 1.0f) * 0.5f * W);
        const int   sy = (int)((1.0f - (clip.reg.m128_f32[1] * inv + 1.0f) * 0.5f) * H);
        const float nz = clip.reg.m128_f32[2] * inv;
        const float radius = std::max(1.0f, p.size * focal_px * inv);

        const float life = p.age / p.lifetime;               // 0 -> 1
        const uint32_t c = lerp_color(p.colorStart, p.colorEnd, life);
        const float alpha = 1.0f - life;                     // fondu de sortie
        const float cr = ((c >> 16) & 0xFF) * alpha;
        const float cg = ((c >> 8) & 0xFF) * alpha;
        const float cb = (c & 0xFF) * alpha;

        const int r = (int)radius + 1;
        const float r2 = radius * radius;
        for (int y = std::max(0, sy - r); y <= std::min(H - 1, sy + r); y++) {
            for (int x = std::max(0, sx - r); x <= std::min(W - 1, sx + r); x++) {
                const float dx = (float)(x - sx), dy = (float)(y - sy);
                if (dx * dx + dy * dy > r2) continue;        // disque
                const int idx = y * W + x;
                if (nz >= _zb[idx]) continue;                // caché par la géométrie (pas d'écriture z)

                const uint32_t d = _fb[idx];
                const uint32_t nr = std::min(255u, ((d >> 16) & 0xFF) + (uint32_t)cr);
                const uint32_t ng = std::min(255u, ((d >> 8) & 0xFF) + (uint32_t)cg);
                const uint32_t nb = std::min(255u, (d & 0xFF) + (uint32_t)cb);
                _fb[idx] = 0xFF000000u | (nr << 16) | (ng << 8) | nb;
            }
        }
    }
}




void Renderer::draw_mesh(const Mesh& mesh,
    const std::vector<Maths::Vec3_SSE>& vertex_normals,
    const Materials& mat,
    const Maths::Matrix4x4_SSE& model,
    const Maths::Matrix4x4_SSE& mvp,
    const Maths::Vec3_SSE& light,
    const Maths::Vec3_SSE& half_vec, bool writeDepth)
{
    Maths::Vec4_SSE point2;
    for (int fidx = 0; fidx < mesh.faceCount(); fidx++) {
        clip_vertex tri[3];
        for (int k = 0; k < 3; k++) {
            int vi = mesh.faces[fidx * 3 + k];
            int ti = mesh.uvFaces[fidx * 3 + k];
            tri[k].clip = mvp * point2.FromPoint(mesh.verts[vi]);
            tri[k].uv = (ti >= 0) ? mesh.uvs[ti] : Maths::UV(0, 0);
            tri[k].normal = model.TransformDirection(vertex_normals[vi]);
        }

        clip_vertex out[6];
        int n_tris = 0;
        clip_triangle_near(tri[0], tri[1], tri[2], out, &n_tris);

        for (int i = 0; i < n_tris; i++) {
            project_and_draw(&out[i * 3], mat, light, half_vec);
        }
    }
}

void Renderer::project_and_draw(const clip_vertex tri[3], const Materials& mat, const Maths::Vec3_SSE& L, const Maths::Vec3_SSE& H)
{

    const int W = _fb.getWidth();
    const int Ht = _fb.getHeight();
    int sx[3], sy[3];
    float nz[3], iw[3];

    for (int k = 0; k < 3; k++) {
        const float inv = 1.0f / tri[k].clip.reg.m128_f32[3];
        sx[k] = (int)((tri[k].clip.reg.m128_f32[0] * inv + 1.0f) * 0.5f * W);
        sy[k] = (int)((1.0f - (tri[k].clip.reg.m128_f32[1] * inv + 1.0f) * 0.5f) * Ht);
        nz[k] = tri[k].clip.reg.m128_f32[2] * inv;
        iw[k] = inv;
    }

    if (_rtz.edge(sx[0], sy[0], sx[1], sy[1], sx[2], sy[2]) >= 0) return;   // back-face cull

    ScreenTri s;
    for (int k = 0; k < 3; k++) {
        s.x[k] = sx[k];  s.y[k] = sy[k];
        s.z[k] = nz[k];  s.iw[k] = iw[k];
        s.uv[k] = tri[k].uv;
        s.n[k] = tri[k].normal;
    }
    s.mat = &mat;  s.L = L;  s.H = H;
    _tris.push_back(s);
}

void Renderer::draw_scene(const std::vector<listeDraw>& scene,const Maths::Vec3_SSE& camera_pos)
{
    for (const auto& items : scene) {
        if (!items.mat.isTransparent())
            draw_mesh(items.mesh, items.vertex_normals, items.mat, items.model, items.mvp, items.light, items.half_vec);
    }

	std::vector<const listeDraw*> transparentObjects;
    for (const auto& items : scene) {
        if (items.mat.isTransparent()) 
            transparentObjects.push_back(&items);
    }

    std::sort(transparentObjects.begin(), transparentObjects.end(),
        [&camera_pos](const listeDraw* a, const listeDraw* b) {
            // Extraction de la position monde (dernière colonne de la matrice modèle)
            Maths::Vec3_SSE posA = get_position(a->model);
            Maths::Vec3_SSE posB = get_position(b->model);

            // Distance au carré (pas besoin de racine carrée pour comparer)
            float distA = posA.DistanceSquared(camera_pos);
            float distB = posB.DistanceSquared(camera_pos);
            
            return distA > distB;
        });

    for (const auto* items : transparentObjects) {
        draw_mesh(items->mesh, items->vertex_normals, items->mat, items->model, items->mvp, items->light, items->half_vec, false);
    }
}