#include "Z_buffer.h"

void ZBuffer::clear_Zbuffer()
{
    for (int i = 0; i < Widths * Heights; i++)
    {
        zbuffer[i] = 1e30f;
    }
}

int ZBuffer::clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

int ZBuffer::edge(int ax, int ay, int bx, int by, int px, int py)
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

void ZBuffer::draw_triangle_zbuffer(int x0, int y0, float z0, float iw0, UV uv0,int x1, int y1, float z1, float iw1, UV uv1,
    int x2, int y2, float z2, float iw2, UV uv2, Rasterizer& rtz, FrameBuffer& fb, const Texture& tex)
{

    int area = edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;                  // degenerate sliver

    // Force one winding so "inside" means "all weights >= 0". We
    // accept BOTH facing directions here (back faces are removed in
    // chapter 8); the Z-buffer is what sorts them out for now, so we
    // must be able to fill a triangle of either winding. Swapping
    // vertices 1 and 2 flips a negative area positive — and the
    // depth z1/z2 must ride along with their vertices.
    if (area < 0) {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(z1, z2);
        std::swap(iw1, iw2);
        std::swap(uv1, uv2);
        area = -area;
    }

    // Bounding box, clamped to the screen.
    int minx = clampi(x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2), 0, Widths - 1);
    int miny = clampi(y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2), 0, Heights - 1);
    int maxx = clampi(x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2), 0, Widths - 1);
    int maxy = clampi(y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2), 0, Heights - 1);

    float inv_area = 1.0f / (float)area;

    // Pre-divide the UVs by w. These (u/w, v/w) are the quantities
   // that DO interpolate linearly in screen space — unlike u, v
   // themselves. 1/w interpolates linearly too.
    float u0w = uv0.x * iw0, v0w = uv0.y * iw0;
    float u1w = uv1.x * iw1, v1w = uv1.y * iw1;
    float u2w = uv2.x * iw2, v2w = uv2.y * iw2;


    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            int e0 = edge(x1, y1, x2, y2, x, y);
            int e1 = edge(x2, y2, x0, y0, x, y);
            int e2 = edge(x0, y0, x1, y1, x, y);
            if ((e0 | e1 | e2) < 0) continue;   // outside the triangle

            // Barycentric weights, normalised to sum to 1.
            float l0 = e0 * inv_area;
            float l1 = e1 * inv_area;
            float l2 = e2 * inv_area;

            // Interpolate depth. NDC z is an AFFINE function of the
            // screen position, so plain screen-space barycentric
            // interpolation is exactly correct for it — no
            // perspective correction needed (that comes in chapter 9,
            // for textures). Smaller z = nearer the camera.
            float z = l0 * z0 + l1 * z1 + l2 * z2;

            int idx = y * Widths + x;

            // Interpolate u/w, v/w, 1/w linearly, then divide back.
            float uw = l0 * u0w + l1 * u1w + l2 * u2w;
            float vw = l0 * v0w + l1 * v1w + l2 * v2w;
            float iw = l0 * iw0 + l1 * iw1 + l2 * iw2;
            float u = uw / iw;
            float v = vw / iw;

            
            if (z >= zbuffer[idx]) continue;   // un pixel plus proche est déjà là
            zbuffer[idx] = z;
            fb[idx] = tex.Sample(u,v);
        }
    }
}



