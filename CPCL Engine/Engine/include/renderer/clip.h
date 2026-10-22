#pragma once
#include <xmmintrin.h>
#include "Vec3_SSE.h"
#include "Vec4_SSE.h"
#include "asset/Mesh.h"   // pour UV

struct clip_vertex {
    Maths::Vec4_SSE clip;     // position clip space (avant division par w)
    Maths::Vec3_SSE normal;   // normale monde
    Maths::UV uv;
};

// r = a + t * (b - a), pour tous les attributs
inline clip_vertex clip_lerp(const clip_vertex& a, const clip_vertex& b, float t)
{
    const __m128 vt = _mm_set1_ps(t);
    clip_vertex r;
    r.clip.reg = _mm_add_ps(a.clip.reg, _mm_mul_ps(vt, _mm_sub_ps(b.clip.reg, a.clip.reg)));
    r.normal.reg = _mm_add_ps(a.normal.reg, _mm_mul_ps(vt, _mm_sub_ps(b.normal.reg, a.normal.reg)));
    r.uv.x = a.uv.x + t * (b.uv.x - a.uv.x);
    r.uv.y = a.uv.y + t * (b.uv.y - a.uv.y);
    return r;
}

// Distance signée au plan near : >= 0 = devant le plan (visible).
// Convention OpenGL : -w <= z  <=>  z + w >= 0
inline float clip_near_dist(const clip_vertex& v)
{
    return v.clip.reg.m128_f32[2] + v.clip.reg.m128_f32[3];
}

// Clippe un triangle contre le plan near.
// out doit pouvoir contenir 6 sommets (2 triangles). *out_n_tris vaut 0, 1 ou 2.
inline void clip_triangle_near(const clip_vertex& v0, const clip_vertex& v1, const clip_vertex& v2,
    clip_vertex* out, int* out_n_tris)
{
    const clip_vertex in[3] = { v0, v1, v2 };
    clip_vertex poly[4];
    int n = 0;

    for (int e = 0; e < 3; e++) {
        const clip_vertex& a = in[e];
        const clip_vertex& b = in[(e + 1) % 3];
        const float da = clip_near_dist(a);
        const float db = clip_near_dist(b);
        const bool a_in = da >= 0.0f;
        const bool b_in = db >= 0.0f;

        if (a_in && b_in) {
            poly[n++] = b;
        }
        else if (a_in && !b_in) {
            poly[n++] = clip_lerp(a, b, da / (da - db));
        }
        else if (!a_in && b_in) {
            poly[n++] = clip_lerp(a, b, da / (da - db));
            poly[n++] = b;
        }
    }

    if (n == 3) {
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        *out_n_tris = 1;
    }
    else if (n == 4) {
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        out[3] = poly[0]; out[4] = poly[2]; out[5] = poly[3];
        *out_n_tris = 2;
    }
    else {
        *out_n_tris = 0;
    }
}
