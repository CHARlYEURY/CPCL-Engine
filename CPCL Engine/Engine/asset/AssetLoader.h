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
    std::vector<Maths::Vec3_SSE> verts;
    std::vector<int> faces;    // 3 ints per face: v0, v1, v2 (0-based)

    int vertCount() const { return (int)verts.size(); }
    int faceCount() const { return (int)faces.size() / 3; }
 }; 

 

// Read the leading integer of a face token like "12" or "12/3/4".
// Returns the integer and advances *p past the token.
inline bool ObjReadIndex(const char*& p, int vertCount, int& out) {
    while (*p == ' ' || *p == '\t') ++p;

    bool negative = false;
    if (*p == '-') { negative = true; ++p; }
    if (*p < '0' || *p > '9') return false;      // plus de token : on s'arrête

    int value = 0;
    while (*p >= '0' && *p <= '9') { value = value * 10 + (*p - '0'); ++p; }

    while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') ++p;

    out = negative ? vertCount - value : value - 1;   // résultat 0-based
    return true;
}

// Load a mesh from an OBJ file. Returns 1 on success, 0 on failure.
inline bool mesh_load(const char* path, Mesh& out) {
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "could not open %s\n", path); return false; }

    out.verts.clear();
    out.faces.clear();


    char line[256];
    while (fgets(line, sizeof line, f)) {

        if (line[0] == 'v' && line[1] == ' ')
        {
            float x, y, z;
            if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3)
            {
                out.verts.push_back(Maths::Vec3_SSE(x, y, z));
            }
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            const char* p = line + 2;
            const int n = out.vertCount();
            int first, prev, cur;

            if (!ObjReadIndex(p, n, first) || !ObjReadIndex(p, n, prev)) continue;

            // triangulation en éventail : gère triangles, quads, polygones
            while (ObjReadIndex(p, n, cur)) {
                if (first >= 0 && first < n && prev >= 0 && prev < n && cur >= 0 && cur < n) {
                    out.faces.push_back(first);
                    out.faces.push_back(prev);
                    out.faces.push_back(cur);
                }
                prev = cur;
            }

        }
        
    }

    fclose(f);
    return !out.verts.empty() && !out.faces.empty();;
}

