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


struct UV
{
    float x, y;
    UV() : x(0), y(0) {}
	UV(float x, float y) : x(x), y(y) {}
};

 struct Mesh {
    std::vector<Maths::Vec3_SSE> verts;
    std::vector<int> faces;    // 3 ints per face: v0, v1, v2 (0-based)
    std::vector<int> uvFaces;    // 3 ints per face: v0, v1, v2 (0-based)
	std::vector<UV> uvs;        // 1 UV per vertex
    int vertCount() const { return (int)verts.size(); }
    int faceCount() const { return (int)faces.size() / 3; }
	int uvCount() const { return (int)uvs.size(); }
 }; 

 

// Read the leading integer of a face token like "12" or "12/3/4".
// Returns the integer and advances *p past the token.
// Lit "v", "v/vt", "v/vt/vn" ou "v//vn". vt = -1 s'il est absent.
 inline bool ObjReadFaceToken(const char*& p, int vertCount, int uvCount, int& outV, int& outVt) {
     while (*p == ' ' || *p == '\t') ++p;
     if (*p == '\0' || *p == '\n' || *p == '\r') return false;

     char* end;
     long v = strtol(p, &end, 10);
     if (end == p) return false;
     p = end;
     outV = (v < 0) ? vertCount + (int)v : (int)v - 1;
     outVt = -1;

     if (*p == '/') {
         ++p;
         if (*p != '/') {                       // il y a un vt
             long t = strtol(p, &end, 10);
             if (end != p) {
                 outVt = (t < 0) ? uvCount + (int)t : (int)t - 1;
                 p = end;
             }
         }
     }
     // on saute le reste (/vn)
     while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') ++p;
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


        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            // a texture coordinate: two floats
            UV t;
            if (sscanf(line + 3, "%f %f", &t.x, &t.y) >= 2) {
                out.uvs.push_back(t);
            }
        }
        else if (line[0] == 'f' && line[1] == ' ')
        {
            const char* p = line + 2;
            const int n = out.vertCount();
            const int nt = out.uvCount();
            int vFirst, tFirst, vPrev, tPrev, vCur, tCur;

            if (!ObjReadFaceToken(p, n, nt, vFirst, tFirst) ||
                !ObjReadFaceToken(p, n, nt, vPrev, tPrev)) continue;

            while (ObjReadFaceToken(p, n, nt, vCur, tCur)) {
                if (vFirst >= 0 && vFirst < n && vPrev >= 0 && vPrev < n && vCur >= 0 && vCur < n) {
                    out.faces.push_back(vFirst);
                    out.faces.push_back(vPrev);
                    out.faces.push_back(vCur);

                    out.uvFaces.push_back(tFirst);
                    out.uvFaces.push_back(tPrev);
                    out.uvFaces.push_back(tCur);
                }
                vPrev = vCur;
                tPrev = tCur;
            }
        }

        
    }

    fclose(f);
    return !out.verts.empty() && !out.faces.empty();;
}

