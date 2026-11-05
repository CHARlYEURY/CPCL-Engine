//  Lit les lignes dont on a besoin :
//    v  x y z      -> un sommet
//    vt u v        -> une s(UV)
//    f				-> une face
//  Le reste (vn, commentaires, matériaux...) est ignoré.
//
//  Les indices d'un .obj commencent à 1 : on retire 1 pour avoir
//  des indices de tableau (base 0). Un indice négatif est accepté
//  (il compte depuis la fin). Un mot de face peut être "i", "i/t",
//  "i/t/n" ou "i//n" ; on lit l'indice du sommet et celui de l'UV.
//
//  Les faces à plus de 3 sommets (quads...) sont découpées en
//  triangles (triangulation en éventail).
// ============================================================

#pragma once
#include "asset/Mesh.h"

// Load a mesh from an OBJ file. Returns 1 on success, 0 on failure.
bool mesh_load(const char* path, Mesh& out);
