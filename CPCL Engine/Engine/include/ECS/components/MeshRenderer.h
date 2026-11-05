// ============================================================
//  MeshRenderer.h  (composant)
// ------------------------------------------------------------
//  dit a l'ECS "cette entité doit etre dessinée, avec CE mesh et CE materiau".
//  une entité sans MeshRenderer est invisible.
//
//  ce sont des POINTEURS vers des assets chargés ailleurs (dans runner) :
//  le composant ne possede rien, ca permet a plusieurs entités de partager le meme mesh
//  et le meme materiau sans les copier. consequence : les assets doivent vivre
//  plus longtemps que les entités qui les utilisent.
//
//  lu par : RenderSystem (il en fait un drawItem pour le Renderer)
// ============================================================

#pragma once
#include "asset/Mesh.h"
#include "asset/textureLoader.h"
#include "asset/Materials.h"


struct MeshRenderer {
    const Mesh* mesh;                                       // la geometrie (sommets, faces, uvs)
    const Materials* materials;                             // couleur / texture / transparence
    const std::vector<Maths::Vec3_SSE>* vertexNormals;      // normales lissées du mesh, calculées une fois au chargement (compute_vertex_normals)
};