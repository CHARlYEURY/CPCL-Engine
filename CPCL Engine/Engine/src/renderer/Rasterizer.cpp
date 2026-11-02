#include "renderer/Rasterizer.h"
#include "renderer/Lighting.h"
#include "core/MathUtils.h"
#include <cmath>
#include <cstdlib>
#include <utility>

// Bresenham "tous octants" : trace un segment en n'utilisant que des entiers.
void Rasterizer::draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);             
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;                   

    for (;;) {
        _mFramebuffer.SetPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }   
        if (e2 <= dx) { err += dx; y0 += sy; }   
    }
}

// Edge function : renvoie 2x l'aire signée du triangle (a, b, p).
// Le signe indique de quel côté de l'arête AB se trouve P.
int Rasterizer::edge(int ax, int ay, int bx, int by, int px, int py) 
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}


// Interpole les couleurs des 3 sommets avec les coordonnées barycentriques.
// e0, e1, e2 = edge functions (poids non normalisés), wsum = e0+e1+e2.
uint32_t Rasterizer::shade(int e0, int e1, int e2, int wsum,
    uint32_t c0, uint32_t c1, uint32_t c2) {

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

// rasterise un triangle avec Z-test, texture perspective-correcte et Phong
// x,y = coords écran (entières) ; z = profondeur NDC ; iw = 1/w (clip space), ça devrait pas être la, mais je me suis emeler les peinceau sur la structure, donc je verrais durant le clean 
void Rasterizer::draw_triangle_phong(const ScreenTri& tri, const TileRect& r, bool writeDepth)
{
    int x0 = tri.x[0], x1 = tri.x[1], x2 = tri.x[2];
    int y0 = tri.y[0], y1 = tri.y[1], y2 = tri.y[2];

    float z0 = tri.z[0], z1 = tri.z[1], z2 = tri.z[2];
    float iw0 = tri.iw[0], iw1 = tri.iw[1], iw2 = tri.iw[2];

    Maths::UV uv0 = tri.uv[0], uv1 = tri.uv[1], uv2 = tri.uv[2];
    Maths::Vec3_SSE n0 = tri.n[0], n1 = tri.n[1], n2 = tri.n[2];

    Materials mat = *tri.mat;

    // dimensions du zbuffer (c'étaient les membres Widths / Heights de ZBuffer)
    const int Widths = _mZBuffer.getWidth();
    const int Heights = _mZBuffer.getHeight();

    // 2x l'aire signée du triangle (v0, v1, v2). Le signe donne son sens de rotation
    int area = edge(x0, y0, x1, y1, x2, y2);
    // aire nulle = triangle plat (3 points alignés) : rien à dessiner
    if (area == 0) return;                 

    // normalise le winding : après ça, "dedans" = les 3 edges >= 0.
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

    // bounding box du triangle, clampée à l'écran
    int minx = clampi(x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2), 0, Widths - 1);
    int miny = clampi(y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2), 0, Heights - 1);
    int maxx = clampi(x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2), 0, Widths - 1);
    int maxy = clampi(y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2), 0, Heights - 1);

    float inv_area = 1.0f / (float)area;        // division par triangle

    // Pre-divide the UVs by w. These (u/w, v/w) are the quantities
    // that DO interpolate linearly in screen space — unlike u, v
    // themselves. 1/w interpolates linearly too.
    float u0w = uv0.x * iw0, v0w = uv0.y * iw0;
    float u1w = uv1.x * iw1, v1w = uv1.y * iw1;
    float u2w = uv2.x * iw2, v2w = uv2.y * iw2;

    Maths::Vec3_SSE n0w = n0 * iw0; 
    Maths::Vec3_SSE n1w = n1 * iw1; 
    Maths::Vec3_SSE n2w = n2 * iw2; 


    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            // poids de chaque arête = aire du sous-triangle opposé au sommet
            int e0 = edge(x1, y1, x2, y2, x, y);
            int e1 = edge(x2, y2, x0, y0, x, y);
            int e2 = edge(x0, y0, x1, y1, x, y);
            if ((e0 | e1 | e2) < 0) continue;   // outside the triangle

            // barycentric weights, normalised to sum to 1.
            float l0 = e0 * inv_area;
            float l1 = e1 * inv_area;
            float l2 = e2 * inv_area;


            // profondeur au pixel mélange des 3 profondeurs (linéaire = correct pour z NDC)
            float z = l0 * z0 + l1 * z1 + l2 * z2;

            int idx = y * Widths + x;

            float iw = l0 * iw0 + l1 * iw1 + l2 * iw2;
            float inv_iw = 1.0f / iw;

            float u = (l0 * u0w + l1 * u1w + l2 * u2w) * inv_iw;
            float v = (l0 * v0w + l1 * v1w + l2 * v2w) * inv_iw;

            // normale au pixel composante par composante, même formule que u et v
            Maths::Vec3_SSE N
                ((l0 * n0w.reg.m128_f32[0] + l1 * n1w.reg.m128_f32[0] + l2 * n2w.reg.m128_f32[0]) * inv_iw,
                (l0 * n0w.reg.m128_f32[1] + l1 * n1w.reg.m128_f32[1] + l2 * n2w.reg.m128_f32[1]) * inv_iw,
                (l0 * n0w.reg.m128_f32[2] + l1 * n1w.reg.m128_f32[2] + l2 * n2w.reg.m128_f32[2]) * inv_iw);

            // l'interpolation raccourcit le vecteur : on le remet à longueur 1 avant l'éclairage
			N = N.Normalize();

            // diffus (Lambert) cos de l'angle normale/lumière, clampé à 0
			float ndot1 = N.Dot(tri.L);
			if (ndot1 < 0) ndot1 = 0;
			float diffuse_k = AMBIENT + (1.0f - AMBIENT) * ndot1;

            // spéculaire (Blinn-Phong) N*H puis puissance (brillance = 16)
			float ndoth = N.Dot(tri.H);
			if (ndoth < 0) ndoth = 0;
			float specular = std::pow(ndoth, 32.f); // Exponent for shininess

			uint32_t texel = mat.sampleBaseColor(u,v);

			uint8_t alpha = (texel >> 24) & 0xFF;
			uint32_t srcColor = color_phong(texel, diffuse_k, specular);

            if (z >= _mZBuffer[idx]) continue;   // un pixel plus proche est déjà là

            if(writeDepth)
                _mZBuffer[idx] = z;

			if (!mat.isTransparent()) {
				_mFramebuffer[idx] = srcColor;
            } 
            else {
                _mFramebuffer[idx] = blend_colors(srcColor, _mFramebuffer[idx], alpha);
            }
        }
    }
}
