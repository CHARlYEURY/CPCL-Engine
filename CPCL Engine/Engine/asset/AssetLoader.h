// ============================================================
//  obj.h — a minimal Wavefront OBJ loader (chapter 6)
// ------------------------------------------------------------
//  Reads the two line types we need:
//    v  x y z      -> a vertex (a point)
//    f  i j k      -> a face (a triangle joining vertices i,j,k)
//  Everything else (vt, vn, comments, ...) is ignored.
//
//  OBJ indices start at 1, so we subtract 1 to make them array
//  indices. Face tokens may be "i", "i/t", or "i/t/n"; we read
//  only the leading vertex index.
//
//  This isn't a complete OBJ parser (no quads, no negative
//  indices, no materials) — just enough for the triangulated
//  teaching meshes we use.
// ============================================================
#pragma once
#include "Vec3_SSE.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
 struct Mesh {
    Maths::Vec3_SSE * verts;     int vert_count;
    int* faces;     int face_count;   // 3 ints per face: v0, v1, v2 (0-based)
}; 

// Read the leading integer of a face token like "12" or "12/3/4".
// Returns the integer and advances *p past the token.
static int obj__read_index(const char** p) {
    // skip spaces
    while (**p == ' ' || **p == '\t') (*p)++;
    int sign = 1;
    if (**p == '-') { sign = -1; (*p)++; }
    int value = 0;
    while (**p >= '0' && **p <= '9') { value = value * 10 + (**p - '0'); (*p)++; }
    // skip the "/texture/normal" part, if any, up to the next space
    while (**p && **p != ' ' && **p != '\t' && **p != '\n' && **p != '\r') (*p)++;
    return sign * value;
}

// Load a mesh from an OBJ file. Returns 1 on success, 0 on failure.
static int mesh_load(const char* path, Mesh* out) {
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "could not open %s\n", path); return 0; }

    //////// INIT ////////
    // Grow-able arrays. Start small, double when full.
    int vcap = 1024, fcap = 1024;
    out->verts = (Maths::Vec3_SSE*)malloc(vcap * sizeof(Maths::Vec3_SSE));
    out->faces = (int*)malloc(fcap * 3 * sizeof(int));
    out->vert_count = 0;
    out->face_count = 0;
    //////// INIT ////////

    char line[256];
    while (fgets(line, sizeof line, f)) {


        //////// sommet /////////
        if (line[0] == 'v' && line[1] == ' ') {
            // a vertex: read three floats
            Maths::Vec3_SSE v;
            if (sscanf(line + 2, "%f %f %f", &v.reg.m128_f32[0], &v.reg.m128_f32[1], &v.reg.m128_f32[2]) == 3) {
                if (out->vert_count == vcap) {
                    vcap *= 2;
                    out->verts = (Maths::Vec3_SSE*)realloc(out->verts, vcap * sizeof(Maths::Vec3_SSE));
                }
                out->verts[out->vert_count++] = v;
            }
        }
        //////// sommet /////////

        //////// face   /////////
        else if (line[0] == 'f' && line[1] == ' ') {
            // a face: read three vertex indices (1-based -> subtract 1)
            const char* p = line + 2;
            int a = obj__read_index(&p) - 1;
            int b = obj__read_index(&p) - 1;
            int c = obj__read_index(&p) - 1;
            if (out->face_count == fcap) {
                fcap *= 2;
                out->faces = (int*)realloc(out->faces, fcap * 3 * sizeof(int));
            }
            out->faces[out->face_count * 3 + 0] = a;
            out->faces[out->face_count * 3 + 1] = b;
            out->faces[out->face_count * 3 + 2] = c;
            out->face_count++;
        }
        //////// face   /////////
        // 
        // any other line type is ignored
    }

    fclose(f);
    return 1;
}

static void mesh_free(Mesh* m) {
    free(m->verts);
    free(m->faces);
    m->verts = NULL;
    m->faces = NULL;
    m->vert_count = 0;
    m->face_count = 0;
}