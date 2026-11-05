// ============================================================
//  Renderer.cpp
// ------------------------------------------------------------
//  implementation du pipeline de rendu (voir Renderer.h pour la vue d'ensemble).
//  ordre d'une frame (appelé depuis RenderSystem) :
//     begin_frame -> draw_particles -> draw_mesh (x chaque objet) -> end_Frame
// ============================================================
#include "renderer/Renderer.h"
#include "Vec4_SSE.h"


// ------------------------------------------------------------
//  lerp_color : mélange deux couleurs canal par canal (r, g, b)
//  t = 0 -> couleur a, t = 1 -> couleur b
//  utilisée seulement par draw_particles pour passer de colorStart a colorEnd
//  pendant la vie de la particule. l'alpha est ignoré (on remet pas de canal a)
//  static = visible que dans ce fichier
// ------------------------------------------------------------
static uint32_t lerp_color(uint32_t a, uint32_t b, float t)
{
    // petite lambda : interpole UN canal, s = decalage en bits du canal (16 = rouge, 8 = vert, 0 = bleu)
    auto ch = [&](int s) {
        float ca = (float)((a >> s) & 0xFF), cb = (float)((b >> s) & 0xFF);
        return (uint32_t)(ca + (cb - ca) * t);
        };
    return (ch(16) << 16) | (ch(8) << 8) | ch(0);
}

// ------------------------------------------------------------
//  begin_frame : remise a zero de tout ce qui vit le temps d'une frame
//  appelé en premier dans RenderSystem
// ------------------------------------------------------------
void Renderer::begin_frame(uint32_t clear_color)
{
    _fb.Clear(clear_color);        // l'image est remplie avec la couleur de fond
    _zb.clear_Zbuffer();           // profondeur remise à « infini »
    _tris.clear();                 // on oublie les triangles de la frame d'avant (la memoire du vector est gardée, pas de realloc)
    for (auto& b : _bins) b.clear();   // pareil pour les listes de chaque tuile
}

// ------------------------------------------------------------
//  end_Frame : c'est ici que le dessin se fait VRAIMENT.
//  jusque la draw_mesh a seulement rempli _tris (liste de triangles deja projetés).
//  2 étapes :
//    1) binning : on regarde pour chaque triangle quelles tuiles il touche
//    2) rasterisation : les threads se partagent les tuiles et dessinent
//       les triangles de chaque tuile avec le rasterizer
//  appelé en dernier dans RenderSystem, juste avant d'envoyer le framebuffer a la fenetre
// ------------------------------------------------------------
void Renderer::end_Frame()
{
    const int W = _fb.getWidth(), H = _fb.getHeight();

    // 1) binning : à quelles tuiles appartient chaque triangle ?
    for (int i = 0; i < (int)_tris.size(); i++) {
        const ScreenTri& s = _tris[i];

        // bounding box du triangle en pixels, clampée a l'ecran
        // (les coordonnées ecran peuvent etre negatives ou depasser, vu qu'on a clippé que le near)
        int minx = std::max(0, std::min({ s.x[0], s.x[1], s.x[2] }));
        int maxx = std::min(W - 1, std::max({ s.x[0], s.x[1], s.x[2] }));
        int miny = std::max(0, std::min({ s.y[0], s.y[1], s.y[2] }));
        int maxy = std::min(H - 1, std::max({ s.y[0], s.y[1], s.y[2] }));
        // box vide apres le clamp = triangle completement hors ecran, on le jette
        if (minx > maxx || miny > maxy) continue;

        // pixel / TILE = numero de tuile. on parcourt toutes les tuiles que la box recouvre
        // et on ajoute l'INDICE du triangle (pas une copie) dans le bin de chaque tuile.
        // un gros triangle se retrouve donc dans plusieurs bins
        for (int ty = miny / TILE; ty <= maxy / TILE; ty++)
            for (int tx = minx / TILE; tx <= maxx / TILE; tx++)
                _bins[ty * _tilesX + tx].push_back(i);
    }

    // 2) rasterisation multi-thread
    // "next" = numero de la prochaine tuile a traiter, partagé entre tous les threads.
    // atomic pour que deux threads ne prennent jamais la meme tuile
    std::atomic<int> next{ 0 };
    const int nTiles = _tilesX * _tilesY;

    // code exécuté par chaque thread : il prend une tuile, la dessine, en reprend une autre, etc.
    // jusqu'a ce qu'il n'y en ait plus
    auto worker = [&]() {
        for (;;) {
            int t = next.fetch_add(1);      // je prends la tuile t et j'incremente le compteur (en une seule operation atomique)
            if (t >= nTiles) break;         // plus de tuile : ce thread a fini

            // numero de tuile -> position (colonne tx, ligne ty)
            int tx = t % _tilesX, ty = t / _tilesX;
            // zone en pixels de la tuile (x1, y1 exclus). min(...) pour la derniere
            // colonne/ligne de tuiles qui peut depasser de l'ecran
            // r = zone de la tuile, transmise au rasterizer (prevue pour limiter le dessin a la tuile)
            TileRect r{ tx * TILE, ty * TILE,
                        std::min((tx + 1) * TILE, W),
                        std::min((ty + 1) * TILE, H)
            };

            // on dessine dans l'ordre tous les triangles de cette tuile (l'ordre d'insertion est gardé)
            // true = on ecrit dans le zbuffer
            for (int i : _bins[t]) {
                _rtz.draw_triangle_phong(_tris[i], r, true);
            }
        }
    };

    // autant de threads que de coeurs logiques, au minimum 1 (hardware_concurrency peut renvoyer 0)
    const int nThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    for (int i = 1; i < nThreads; i++) pool.emplace_back(worker);   // nThreads-1 threads en plus du principal
    worker();                                  // le thread principal travaille aussi
    for (auto& th : pool) th.join();           // on attend que tout soit fini
}

// ------------------------------------------------------------
//  draw_particles : dessine toutes les particules du ParticleSystem.
//  contrairement aux meshs, on passe PAS par _tris : chaque particule est un petit
//  disque dessiné tout de suite dans le framebuffer, en mode additif (la couleur
//  s'ajoute au pixel => effet "feu / lumiere").
//  vp = proj * view  (les particules ont deja une position monde)
//  focal_px = focale en pixels, pour que la taille des particules suive la perspective
//  appelé par RenderSystem juste apres begin_frame
// ------------------------------------------------------------
void Renderer::draw_particles(const ParticleSystem& ps, const Maths::Matrix4x4_SSE& vp, float focal_px)
{
    const int W = _fb.getWidth(), H = _fb.getHeight();
    Maths::Vec4_SSE tmp;     // sert juste a appeler FromPoint (construit un Vec4 avec w = 1)

    for (const Particle& p : ps.particles) {
        // monde -> clip space
        Maths::Vec4_SSE clip = vp * tmp.FromPoint(p.pos);
        const float w = clip.reg.m128_f32[3];
        if (w < 0.1f) continue;                              // derrière / trop près de la caméra

        // division perspective (clip -> ndc) puis ndc [-1,1] -> pixels
        // le y est inversé car en ecran l'origine est en haut a gauche
        const float inv = 1.0f / w;
        const int   sx = (int)((clip.reg.m128_f32[0] * inv + 1.0f) * 0.5f * W);
        const int   sy = (int)((1.0f - (clip.reg.m128_f32[1] * inv + 1.0f) * 0.5f) * H);
        const float nz = clip.reg.m128_f32[2] * inv;         // profondeur ndc, comparée au zbuffer plus bas
        // taille en pixels : taille monde * focale / distance (w ~ distance) => plus c'est loin plus c'est petit
        // minimum 1 pixel pour pas que les particules lointaines disparaissent
        const float radius = std::max(1.0f, p.size * focal_px * inv);

        // avancement dans la vie de la particule : 0 = elle vient de naitre, 1 = elle va mourir
        const float life = p.age / p.lifetime;               // 0 -> 1
        const uint32_t c = lerp_color(p.colorStart, p.colorEnd, life);   // la couleur change au fil du temps
        const float alpha = 1.0f - life;                     // fondu de sortie
        // couleur déja multipliée par alpha : comme on additionne, une valeur faible = particule presque invisible
        const float cr = ((c >> 16) & 0xFF) * alpha;
        const float cg = ((c >> 8) & 0xFF) * alpha;
        const float cb = (c & 0xFF) * alpha;

        // on parcourt le carré qui entoure le disque, clampé aux bords de l'ecran
        const int r = (int)radius + 1;
        const float r2 = radius * radius;     // on compare des distances au carré (pas de sqrt)
        for (int y = std::max(0, sy - r); y <= std::min(H - 1, sy + r); y++) {
            for (int x = std::max(0, sx - r); x <= std::min(W - 1, sx + r); x++) {
                const float dx = (float)(x - sx), dy = (float)(y - sy);
                if (dx * dx + dy * dy > r2) continue;        // disque
                const int idx = y * W + x;
                if (nz >= _zb[idx]) continue;                // caché par la géométrie (pas d'écriture z)

                // blending additif : nouveau pixel = ancien + couleur particule, plafonné a 255
                const uint32_t d = _fb[idx];
                const uint32_t nr = std::min(255u, ((d >> 16) & 0xFF) + (uint32_t)cr);
                const uint32_t ng = std::min(255u, ((d >> 8) & 0xFF) + (uint32_t)cg);
                const uint32_t nb = std::min(255u, (d & 0xFF) + (uint32_t)cb);
                _fb[idx] = 0xFF000000u | (nr << 16) | (ng << 8) | nb;
            }
        }
    }
}

// ------------------------------------------------------------
//  draw_mesh : traite tous les triangles d'un objet (etapes "geometrie" du pipeline).
//  pour chaque face :
//    1) on transforme les 3 sommets en clip space avec la mvp
//    2) on recupere uv et normale de chaque sommet
//    3) on clippe le triangle contre le plan near (ca peut donner 0, 1 ou 2 triangles)
//    4) chaque triangle obtenu part dans project_and_draw
//  rien n'est dessiné ici, on remplit juste _tris. le vrai dessin est dans end_Frame.
//  appelé par RenderSystem (une fois par entité) et par draw_scene
// ------------------------------------------------------------
void Renderer::draw_mesh(const drawItem& item, bool writeDepth)
{

    Maths::Vec4_SSE point2;    // temporaire pour FromPoint (position 3d -> vec4 avec w = 1)
    for (int fidx = 0; fidx < item.mesh.faceCount(); fidx++) {
        clip_vertex tri[3];
        for (int k = 0; k < 3; k++) {
            int vi = item.mesh.faces[fidx * 3 + k];      // indice du sommet dans mesh.verts
            int ti = item.mesh.uvFaces[fidx * 3 + k];    // indice de l'uv dans mesh.uvs (-1 si le face n'a pas de vt)
            tri[k].clip = item.mvp * point2.FromPoint(item.mesh.verts[vi]);   // local -> clip space
            tri[k].uv = (ti >= 0) ? item.mesh.uvs[ti] : Maths::UV(0, 0);      // pas d'uv = (0,0) par defaut
            // la normale est tournée par la matrice model pour etre en espace monde
            // (la lumiere est en monde aussi, sinon le calcul de l'eclairage serait faux)
            tri[k].normal = item.model.TransformDirection(item.vertex_normals[vi]);
        }

        // clipping near : out peut contenir jusqu'a 2 triangles (6 sommets)
        clip_vertex out[6];
        int n_tris = 0;
        clip_triangle_near(tri[0], tri[1], tri[2], out, &n_tris);

        // n_tris = 0 -> triangle completement derriere la camera, la boucle ne tourne pas
        for (int i = 0; i < n_tris; i++) {
            project_and_draw(&out[i * 3], item);
        }
    }
}

// ------------------------------------------------------------
//  project_and_draw : derniere etape avant la rasterisation.
//  prend un triangle deja clippé (clip space) et :
//    1) fait la division par w (perspective) et passe en coordonnées pixels
//    2) jette le triangle s'il tourne le dos a la camera (back-face culling)
//    3) range le resultat dans un ScreenTri pousé dans _tris
//  appelé par draw_mesh pour chaque triangle sorti du clipping
// ------------------------------------------------------------
void Renderer::project_and_draw(const clip_vertex tri[3], const drawItem& item)
{

    const int W = _fb.getWidth();
    const int Ht = _fb.getHeight();
    int sx[3], sy[3];       // position ecran en pixels (entiers, c'est ce qu'utilise le rasterizer)
    float nz[3], iw[3];     // profondeur ndc et 1/w de chaque sommet

    for (int k = 0; k < 3; k++) {
        // division perspective : clip -> ndc. on calcule 1/w une seule fois et on multiplie
        const float inv = 1.0f / tri[k].clip.reg.m128_f32[3];
        // ndc [-1,1] -> pixels. y inversé (l'ecran a son origine en haut)
        sx[k] = (int)((tri[k].clip.reg.m128_f32[0] * inv + 1.0f) * 0.5f * W);
        sy[k] = (int)((1.0f - (tri[k].clip.reg.m128_f32[1] * inv + 1.0f) * 0.5f) * Ht);
        nz[k] = tri[k].clip.reg.m128_f32[2] * inv;    // z ndc, interpolé linéairement par le rasterizer pour le zbuffer
        iw[k] = inv;    // on garde 1/w : necessaire pour interpoler uv et normale en perspective correcte
    }

    // back-face culling : l'aire signée du triangle en pixels donne son sens de rotation.
    // avec le y inversé, une face visible a une aire négative, donc >= 0 = de dos (ou plat) -> on jette
    if (_rtz.edge(sx[0], sy[0], sx[1], sy[1], sx[2], sy[2]) >= 0) return;   // back-face cull

    // on recopie tout dans un ScreenTri, format que comprend le rasterizer
    ScreenTri s;
    for (int k = 0; k < 3; k++) {
        s.x[k] = sx[k];  s.y[k] = sy[k];
        s.z[k] = nz[k];  s.iw[k] = iw[k];
        s.uv[k] = tri[k].uv;
        s.n[k] = tri[k].normal;
    }
    // infos communes aux 3 sommets : materiau (pointeur, l'objet doit rester vivant jusqu'a end_Frame),
    // direction de lumiere et half vector (copiés)
    s.mat = &item.mat;  s.L = item.light;  s.H = item.half_vec;
    _tris.push_back(s);     // sera dessiné plus tard dans end_Frame
}

// ------------------------------------------------------------
//  draw_scene : dessine une liste d'objets en gérant la transparence.
//  principe classique :
//    1) les objets opaques d'abord (ordre sans importance grace au zbuffer)
//    2) les transparents ensuite, triés du plus LOIN au plus PROCHE, sans ecrire dans
//       le zbuffer, pour que le blending se fasse dans le bon ordre
//  camera_pos = position de la camera, sert au tri
//  (pour l'instant RenderSystem appelle draw_mesh directement, cette fonction est la version "scene complete")
// ------------------------------------------------------------
void Renderer::draw_scene(const std::vector<drawItem>& scene, const Maths::Vec3_SSE& camera_pos)
{
    // passe 1 : opaques
    for (const auto& items : scene) {
        if (!items.mat.isTransparent())
            draw_mesh(items);
    }

    // passe 2 : on met les transparents de coté dans une liste de pointeurs (pas de copie de drawItem)
    std::vector<const drawItem*> transparentObjects;
    for (const auto& items : scene) {
        if (items.mat.isTransparent())
            transparentObjects.push_back(&items);
    }

    // tri decroissant sur la distance = le plus loin en premier (painter's algorithm)
    std::sort(transparentObjects.begin(), transparentObjects.end(),
        [&camera_pos](const drawItem* a, const drawItem* b) {
            // Extraction de la position monde (dernière colonne de la matrice modèle)
            Maths::Vec3_SSE posA = get_position(a->model);
            Maths::Vec3_SSE posB = get_position(b->model);

            // Distance au carré (pas besoin de racine carrée pour comparer)
            float distA = posA.DistanceSquared(camera_pos);
            float distB = posB.DistanceSquared(camera_pos);

            return distA > distB;
        });

    // writeDepth = false : un objet transparent ne doit pas cacher ceux qui sont derriere lui
    for (const auto* items : transparentObjects) {
        draw_mesh(*items, false);
    }
}