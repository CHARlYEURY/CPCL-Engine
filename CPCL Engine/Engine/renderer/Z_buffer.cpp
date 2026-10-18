#include "Z_buffer.h"

#define AMBIENT 0.18f

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

float ZBuffer::pow32(float x)
{
    float x2 = x * x;
    float x4 = x2 * x2;
    float x8 = x4 * x4;
    float x16 = x8 * x8;
    return x16 * x16;
}

uint32_t ZBuffer::color_phong(uint32_t texel, float diffuse_k, float spec_k)
{
    float r = (float)((texel >> 16) & 0xFF) * diffuse_k + 255.0f * spec_k;
    float g = (float)((texel >> 8) & 0xFF) * diffuse_k + 255.0f * spec_k;
    float b = (float)(texel & 0xFF) * diffuse_k + 255.0f * spec_k;
    if (r > 255.0f) r = 255.0f;
    if (g > 255.0f) g = 255.0f;
    if (b > 255.0f) b = 255.0f;
    return ((uint32_t)0xFFu << 24)
        | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}


uint32_t ZBuffer::color_scale(uint32_t c, float k)
{
    if (k < 0.0f) k = 0.0f;
    if (k > 1.0f) k = 1.0f;
    uint32_t r = (uint32_t)((float)((c >> 16) & 0xFF) * k);
    uint32_t g = (uint32_t)((float)((c >> 8) & 0xFF) * k);
    uint32_t b = (uint32_t)((float)(c & 0xFF) * k);
    return ((uint32_t)0xFFu << 24) | (r << 16) | (g << 8) | b;
}

void ZBuffer::compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3_SSE>& face_normals, std::vector<Maths::Vec3_SSE>& out)
{
    const int vertCount = mesh.vertCount();
    const int faceCount = mesh.faceCount();

    // 1. Tout remettre à zéro (resize + assign évite les anciennes valeurs)
    out.assign(vertCount, Maths::Vec3_SSE(0, 0, 0));

    // 2. Chaque triangle ajoute sa normale à ses 3 sommets
    for (int f = 0; f < faceCount; f++) {
        for (int k = 0; k < 3; k++) {
            int v = mesh.faces[f * 3 + k];
            out[v] = out[v] + face_normals[f];
        }
    }

    // 3. Normaliser la somme (= moyenne des normales des faces adjacentes)
    for (int i = 0; i < vertCount; i++) {
        out[i] = out[i].Normalize();
    }
}

void ZBuffer::compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3_SSE>& out)
{
    const int faceCount = mesh.faceCount();
    out.resize(faceCount);

    for (int fidx = 0; fidx < faceCount; fidx++) {
        int a = mesh.faces[fidx * 3 + 0];
        int b = mesh.faces[fidx * 3 + 1];
        int c = mesh.faces[fidx * 3 + 2];

        const Maths::Vec3_SSE& v0 = mesh.verts[a];
        const Maths::Vec3_SSE& v1 = mesh.verts[b];
        const Maths::Vec3_SSE& v2 = mesh.verts[c];

        Maths::Vec3_SSE e1 = v1 - v0;
        Maths::Vec3_SSE e2 = v2 - v0;
        out[fidx] = e1.Cross(e2).Normalize();   // une normale PAR triangle
    }
}




void ZBuffer::draw_triangle_zbuffer(
    int x0, int y0, float z0, float iw0, UV uv0, Maths::Vec3_SSE n0,
    int x1, int y1, float z1, float iw1, UV uv1, Maths::Vec3_SSE n1,
    int x2, int y2, float z2, float iw2, UV uv2, Maths::Vec3_SSE n2, Rasterizer& rtz, FrameBuffer& fb, const Texture& tex, Maths::Vec3_SSE L, Maths::Vec3_SSE H)
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
        std::swap(n1, n2);     
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
    Maths::Vec3_SSE n0w = n0 * iw0; 
    Maths::Vec3_SSE n1w = n1 * iw1; 
    Maths::Vec3_SSE n2w = n2 * iw2; 


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


            float iw = l0 * iw0 + l1 * iw1 + l2 * iw2;
            float inv_iw = 1.0f / iw;

            float u = (l0 * u0w + l1 * u1w + l2 * u2w) * inv_iw;
            float v = (l0 * v0w + l1 * v1w + l2 * v2w) * inv_iw;

            Maths::Vec3_SSE N
                ((l0 * n0w.reg.m128_f32[0] + l1 * n1w.reg.m128_f32[0] + l2 * n2w.reg.m128_f32[0]) * inv_iw,
                (l0 * n0w.reg.m128_f32[1] + l1 * n1w.reg.m128_f32[1] + l2 * n2w.reg.m128_f32[1]) * inv_iw,
                (l0 * n0w.reg.m128_f32[2] + l1 * n1w.reg.m128_f32[2] + l2 * n2w.reg.m128_f32[2]) * inv_iw);

			N = N.Normalize();

			float ndot1 = N.Dot(L);
			if (ndot1 < 0) ndot1 = 0;
			float diffuse_k = AMBIENT + (1.0f - AMBIENT) * ndot1;

			float ndoth = N.Dot(H);
			if (ndoth < 0) ndoth = 0;
			float specular = std::pow(ndoth, 16.0f); // Exponent for shininess

			uint32_t texel = tex.Sample(u,v);
            
            if (z >= zbuffer[idx]) continue;   // un pixel plus proche est déjà là
            zbuffer[idx] = z;
            fb[idx] = color_phong(texel, diffuse_k, specular);
        }
    }
}



