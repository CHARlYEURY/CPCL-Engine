#pragma once 
#include "Vec3_SSE.h"
#include "Vec4_SSE.h"
#include <stdbool.h>
#include <vector>


struct clip_vertex {
	Maths::Vec4_SSE clip;
	Maths::Vec3_SSE normal; 
	Maths::UV uv;
};

clip_vertex clip_lerp(clip_vertex a, clip_vertex b, float t)
{
    clip_vertex r;
    r.clip.reg.m128_f32[0] = a.clip.reg.m128_f32[0] + t * (b.clip.reg.m128_f32[0] - a.clip.reg.m128_f32[0]);
    r.clip.reg.m128_f32[1] = a.clip.reg.m128_f32[1] + t * (b.clip.reg.m128_f32[1] - a.clip.reg.m128_f32[1]);
    r.clip.reg.m128_f32[2] = a.clip.reg.m128_f32[2] + t * (b.clip.reg.m128_f32[2] - a.clip.reg.m128_f32[2]);
    r.clip.reg.m128_f32[3] = a.clip.reg.m128_f32[3] + t * (b.clip.reg.m128_f32[3] - a.clip.reg.m128_f32[3]);
    r.uv.x = a.uv.x + t * (b.uv.x - a.uv.x);
    r.uv.y = a.uv.y + t * (b.uv.y - a.uv.y);
    r.normal.reg.m128_f32[0] = a.normal.reg.m128_f32[0] + t * (b.normal.reg.m128_f32[0] - a.normal.reg.m128_f32[0]);
    r.normal.reg.m128_f32[1] = a.normal.reg.m128_f32[1] + t * (b.normal.reg.m128_f32[1] - a.normal.reg.m128_f32[1]);
    r.normal.reg.m128_f32[2] = a.normal.reg.m128_f32[2] + t * (b.normal.reg.m128_f32[2] - a.normal.reg.m128_f32[2]);
    return r;
}



float clip__near_dist(clip_vertex v) {
    return v.clip.reg.m128_f32[0] + v.clip.reg.m128_f32[3];
}


void clip_triangle_near(clip_vertex v0, clip_vertex v1, clip_vertex v2,
    clip_vertex* out, int* out_n_tris) {
    clip_vertex in[3] = { v0, v1, v2 };
    clip_vertex poly[4];          // 3 in -> at most 4 out for one plane
    int n = 0;

    // Walk the edges v0->v1, v1->v2, v2->v0. For each, append the
    // inside endpoint and/or the crossing point to the output polygon.
    for (int e = 0; e < 3; e++) {
        clip_vertex a = in[e];
        clip_vertex b = in[(e + 1) % 3];
        float da = clip__near_dist(a);
        float db = clip__near_dist(b);
        bool a_in = da >= 0.0f;
        bool b_in = db >= 0.0f;

        if (a_in && b_in) {
            poly[n++] = b;                       // both in: emit b
        }
        else if (a_in && !b_in) {
            float t = da / (da - db);            // leaving: emit crossing
            poly[n++] = clip_lerp(a, b, t);
        }
        else if (!a_in && b_in) {
            float t = da / (da - db);            // entering: crossing then b
            poly[n++] = clip_lerp(a, b, t);
            poly[n++] = b;
        }
        // both out: emit nothing
    }

    if (n == 3) {
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        *out_n_tris = 1;
    }
    else if (n == 4) {
        // Fan-triangulate the quad: (0,1,2) and (0,2,3).
        out[0] = poly[0]; out[1] = poly[1]; out[2] = poly[2];
        out[3] = poly[0]; out[4] = poly[2]; out[5] = poly[3];
        *out_n_tris = 2;
    }
    else {
        *out_n_tris = 0;
    }
}
