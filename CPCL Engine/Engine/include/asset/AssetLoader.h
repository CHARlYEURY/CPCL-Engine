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
#include "asset/Mesh.h"

// Load a mesh from an OBJ file. Returns 1 on success, 0 on failure.
bool mesh_load(const char* path, Mesh& out);
