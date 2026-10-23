#include "renderer/Renderer.h"
#include "Vec4_SSE.h"

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
                    *s.tex, s.L, s.H, r);
            }
        }
        };

    const int nThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    for (int i = 1; i < nThreads; i++) pool.emplace_back(worker);
    worker();                                  // le thread principal travaille aussi
    for (auto& th : pool) th.join();           // on attend que tout soit fini
}

void Renderer::draw_mesh(const Mesh& mesh,
    const std::vector<Maths::Vec3_SSE>& vertex_normals,
    const Texture& tex,
    const Maths::Matrix4x4_SSE& model,
    const Maths::Matrix4x4_SSE& mvp,
    const Maths::Vec3_SSE& light,
    const Maths::Vec3_SSE& half_vec)
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
            project_and_draw(&out[i * 3], tex, light, half_vec);
        }
    }
}

void Renderer::project_and_draw(const clip_vertex tri[3], const Texture& tex, const Maths::Vec3_SSE& L, const Maths::Vec3_SSE& H)
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
    s.tex = &tex;  s.L = L;  s.H = H;
    _tris.push_back(s);
}
