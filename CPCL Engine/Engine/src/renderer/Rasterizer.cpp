// ============================================================
//  Rasterizer.cpp
// ------------------------------------------------------------
//  implementation du rasterizer (voir Rasterizer.h pour la vue d'ensemble).
//  la fonction importante c'est draw_triangle_phong, le reste c'est des outils
//  (edge, draw_line) ou de l'ancien code gardé (shade).
// ============================================================
#include "renderer/Rasterizer.h"
#include "renderer/Lighting.h"       
#include "core/MathUtils.h"          
#include <cmath>
#include <cstdlib>
#include <utility>

// ------------------------------------------------------------
//  draw_line : trace un segment entre (x0,y0) et (x1,y1)
//  Bresenham "tous octants" : n'utilise que des entiers, pas de division ni de float.
//  principe : err garde l'ecart entre la vraie droite et le pixel choisi,
//  a chaque pixel on decide si on avance en x, en y, ou les deux.
//  utilisé pour du debug (fil de fer), pas dans le rendu normal
// ------------------------------------------------------------
void Rasterizer::draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = abs(x1 - x0);              // distance en x (toujours positive)
    int dy = -abs(y1 - y0);             // distance en y mise en NEGATIF (astuce de la version tous octants)
    int sx = (x0 < x1) ? 1 : -1;        // sens de deplacement en x
    int sy = (y0 < y1) ? 1 : -1;        // sens de deplacement en y
    int err = dx + dy;                  // erreur de depart

    for (;;) {
        _mFramebuffer.SetPixel(x0, y0, color);      // SetPixel verifie deja les bords de l'ecran
        if (x0 == x1 && y0 == y1) break;            // on est arrivé au bout
        int e2 = 2 * err;                           // 2*err pour eviter de travailler avec des demis
        if (e2 >= dy) { err += dy; x0 += sx; }     // l'erreur dit qu'il faut avancer en x
        if (e2 <= dx) { err += dx; y0 += sy; }     // l'erreur dit qu'il faut avancer en y
        // (si les deux sont vrai on avance en diagonale)
    }
}

// ------------------------------------------------------------
//  edge : edge function entre l'arete a->b et le point p
//  Edge function : renvoie 2x l'aire signée du triangle (a, b, p).
//  Le signe indique de quel côté de l'arête AB se trouve P.
//  c'est un produit en croix 2d : > 0 d'un coté, < 0 de l'autre, 0 si p est pile sur la droite.
//  trois usages :
//    - savoir si un pixel est dans un triangle (les 3 edges de meme signe)
//    - donner les poids barycentriques (chaque edge = aire du sous-triangle)
//    - donner le sens de rotation d'un triangle (back-face culling dans le Renderer)
// ------------------------------------------------------------
int Rasterizer::edge(int ax, int ay, int bx, int by, int px, int py)
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}


// ------------------------------------------------------------
//  shade : ancienne version, interpolation de couleur par sommet (gouraud)
//  Interpole les couleurs des 3 sommets avec les coordonnées barycentriques.
//  e0, e1, e2 = edge functions (poids non normalisés), wsum = e0+e1+e2.
//  plus utilisée maintenant : on calcule l'eclairage a chaque pixel dans draw_triangle_phong
// ------------------------------------------------------------
uint32_t Rasterizer::shade(int e0, int e1, int e2, int wsum,
    uint32_t c0, uint32_t c1, uint32_t c2) {

    // pour chaque canal : somme ponderée des 3 sommets, divisée par la somme des poids
    uint8_t r = (uint8_t)((e0 * ((c0 >> 16) & 0xFF)
        + e1 * ((c1 >> 16) & 0xFF)
        + e2 * ((c2 >> 16) & 0xFF)) / wsum);
    uint8_t g = (uint8_t)((e0 * ((c0 >> 8) & 0xFF)
        + e1 * ((c1 >> 8) & 0xFF)
        + e2 * ((c2 >> 8) & 0xFF)) / wsum);
    uint8_t b = (uint8_t)((e0 * (c0 & 0xFF)
        + e1 * (c1 & 0xFF)
        + e2 * (c2 & 0xFF)) / wsum);
    return setRGB(r, g, b);
}

// ------------------------------------------------------------
//  draw_triangle_phong : LA fonction principale du rasterizer.
//  rasterise un triangle avec Z-test, texture perspective-correcte et Phong
//  x,y = coords écran (entières) ; z = profondeur NDC ; iw = 1/w (clip space), ça devrait pas être la, mais je me suis emeler les peinceau sur la structure, donc je verrais durant le clean 
//
//  etapes :
//    1) on recopie les données du triangle dans des variables locales
//    2) on calcule l'aire, on rejette les triangles plats, on remet le triangle dans le bon sens
//    3) on calcule la bounding box (zone de pixels a tester)
//    4) on prepare les valeurs divisées par w (pour la perspective correcte)
//    5) pour chaque pixel de la box :
//         - est il dans le triangle ?  (edge functions)
//         - on interpole z, uv, normale
//         - on calcule l'eclairage et la couleur
//         - z-test, puis ecriture dans le framebuffer (opaque ou blendé)
//
//  appelée par Renderer::end_Frame, une fois par triangle et par tuile touchée
//  tri = triangle a dessiner / r = tuile courante / writeDepth = ecrire ou non dans le zbuffer
// ------------------------------------------------------------
void Rasterizer::draw_triangle_phong(const ScreenTri& tri, const TileRect& r, bool writeDepth)
{
    // 1) copie locale des attributs (plus court a ecrire et ca permet de les swap plus bas)
    int x0 = tri.x[0], x1 = tri.x[1], x2 = tri.x[2];
    int y0 = tri.y[0], y1 = tri.y[1], y2 = tri.y[2];

    float z0 = tri.z[0], z1 = tri.z[1], z2 = tri.z[2];
    float iw0 = tri.iw[0], iw1 = tri.iw[1], iw2 = tri.iw[2];

    Maths::UV uv0 = tri.uv[0], uv1 = tri.uv[1], uv2 = tri.uv[2];
    Maths::Vec3_SSE n0 = tri.n[0], n1 = tri.n[1], n2 = tri.n[2];

    // attention : ici c'est une COPIE du materiau (pas une reference)
    Materials mat = *tri.mat;

    // dimensions du zbuffer (c'étaient les membres Widths / Heights de ZBuffer)
    const int Widths = _mZBuffer.getWidth();
    const int Heights = _mZBuffer.getHeight();

    // 2) 2x l'aire signée du triangle (v0, v1, v2). Le signe donne son sens de rotation
    int area = edge(x0, y0, x1, y1, x2, y2);
    // aire nulle = triangle plat (3 points alignés) : rien à dessiner
    if (area == 0) return;

    // normalise le winding : après ça, "dedans" = les 3 edges >= 0.
    // si le triangle est dans l'autre sens on echange les sommets 1 et 2
    // (et TOUS leurs attributs, sinon les uv/normales ne correspondraient plus aux bons points)
    if (area < 0) {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(z1, z2);
        std::swap(iw1, iw2);
        std::swap(uv1, uv2);
        std::swap(n1, n2);
        area = -area;
    }

    // a partir d'ici un pixel est dasn le triangle si ses 3 edge functions sont >= 0

    // 3) bounding box du triangle, clampée à l'écran
    // (min / max des 3 coordonnées a la main avec des ternaires, puis clampi pour rester dans l'image)
    int minx = clampi(x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2), 0, Widths - 1);
    int miny = clampi(y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2), 0, Heights - 1);
    int maxx = clampi(x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2), 0, Widths - 1);
    int maxy = clampi(y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2), 0, Heights - 1);

    float inv_area = 1.0f / (float)area;        // division par triangle (on la fait une seule fois, apres on multiplie)

    // 4) perspective correcte
    // Pre-divide the UVs by w. These (u/w, v/w) are the quantities
    // that DO interpolate linearly in screen space — unlike u, v
    // themselves. 1/w interpolates linearly too.
    // en francais : a l'ecran u et v s'interpolent PAS lineairement (perspective), par contre u/w, v/w et 1/w oui.
    // donc on interpole ces valeurs la, puis on divise par 1/w interpolé pour retrouver le vrai u et v
    float u0w = uv0.x * iw0, v0w = uv0.y * iw0;
    float u1w = uv1.x * iw1, v1w = uv1.y * iw1;
    float u2w = uv2.x * iw2, v2w = uv2.y * iw2;

    // pareil pour les normales : on les divise par w avant l'interpolation
    Maths::Vec3_SSE n0w = n0 * iw0;
    Maths::Vec3_SSE n1w = n1 * iw1;
    Maths::Vec3_SSE n2w = n2 * iw2;


    // 5) boucle sur tous les pixels de la bounding box
    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            // poids de chaque arête = aire du sous-triangle opposé au sommet
            // e0 est lié au sommet 0 (arete 1->2), e1 au sommet 1, e2 au sommet 2
            int e0 = edge(x1, y1, x2, y2, x, y);
            int e1 = edge(x2, y2, x0, y0, x, y);
            int e2 = edge(x0, y0, x1, y1, x, y);
            // astuce : si un des trois est negatif, le bit de signe du "ou" est a 1 -> pixel dehors
            // (>= 0 donc un pixel pile sur une arete est considéré dedans)
            if ((e0 | e1 | e2) < 0) continue;   // outside the triangle

            // barycentric weights, normalised to sum to 1.
            // l0 + l1 + l2 = 1, c'est "combien chaque sommet compte" pour ce pixel
            float l0 = e0 * inv_area;
            float l1 = e1 * inv_area;
            float l2 = e2 * inv_area;


            // profondeur au pixel mélange des 3 profondeurs (linéaire = correct pour z NDC)
            float z = l0 * z0 + l1 * z1 + l2 * z2;

            int idx = y * Widths + x;     // position du pixel dans les tableaux 1d (framebuffer et zbuffer)

            // 1/w interpolé, puis on l'inverse pour retrouver w (sert a "defaire" la division par w de plus haut)
            float iw = l0 * iw0 + l1 * iw1 + l2 * iw2;
            float inv_iw = 1.0f / iw;

            // uv perspective-correct : (u/w interpolé) / (1/w interpolé)
            float u = (l0 * u0w + l1 * u1w + l2 * u2w) * inv_iw;
            float v = (l0 * v0w + l1 * v1w + l2 * v2w) * inv_iw;

            // normale au pixel composante par composante, même formule que u et v
            // (m128_f32[0..2] = x, y, z du Vec3_SSE)
            Maths::Vec3_SSE N
            ((l0 * n0w.reg.m128_f32[0] + l1 * n1w.reg.m128_f32[0] + l2 * n2w.reg.m128_f32[0]) * inv_iw,
                (l0 * n0w.reg.m128_f32[1] + l1 * n1w.reg.m128_f32[1] + l2 * n2w.reg.m128_f32[1]) * inv_iw,
                (l0 * n0w.reg.m128_f32[2] + l1 * n1w.reg.m128_f32[2] + l2 * n2w.reg.m128_f32[2]) * inv_iw);

            // l'interpolation raccourcit le vecteur : on le remet à longueur 1 avant l'éclairage
            N = N.Normalize();

            // diffus (Lambert) cos de l'angle normale/lumière, clampé à 0
            // (si la face regarde a l'opposé de la lumiere le produit scalaire est negatif -> 0)
            float ndot1 = N.Dot(tri.L);
            if (ndot1 < 0) ndot1 = 0;
            // on ajoute la lumiere ambiante pour que les zones dans l'ombre soient pas completement noires
            float diffuse_k = AMBIENT + (1.0f - AMBIENT) * ndot1;

            // spéculaire (Blinn-Phong) N*H puis puissance (brillance = 16)
            // (dans le code l'exposant est en fait 32, le commentaire d'origine disait 16)
            float ndoth = N.Dot(tri.H);
            if (ndoth < 0) ndoth = 0;
            float specular = std::pow(ndoth, 32.f); // Exponent for shininess

            // couleur de base : texture au point (u,v) multipliée par la couleur diffuse du materiau
            uint32_t texel = mat.sampleBaseColor(u, v);

            // on garde l'alpha du texel (pour le blending plus bas)
            uint8_t alpha = (texel >> 24) & 0xFF;
            // couleur finale du pixel eclairé : texel * diffus + blanc * speculaire
            uint32_t srcColor = color_phong(texel, diffuse_k, specular);

            // z-test : plus petit = plus proche de la camera
            if (z >= _mZBuffer[idx]) continue;   // un pixel plus proche est déjà là

            // pour les transparents writeDepth devrait etre false : ils cachent pas ce qu'il y a derriere
            if (writeDepth)
                _mZBuffer[idx] = z;

            // opaque : on remplace le pixel. transparent : on melange avec ce qui est deja dans le framebuffer
            if (!mat.isTransparent()) {
                _mFramebuffer[idx] = srcColor;
            }
            else {
                _mFramebuffer[idx] = blend_colors(srcColor, _mFramebuffer[idx], alpha);
            }
        }
    }
}