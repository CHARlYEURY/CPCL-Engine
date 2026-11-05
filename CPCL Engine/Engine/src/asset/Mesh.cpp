#include "asset/Mesh.h"

//  Le mesh ne stocke pas les normales, on les calcule ici en 2 étapes :
//    1) compute_face_normals   : une normale par triangle
//    2) compute_vertex_normals : une normale par sommet (moyenne des
//       normales des triangles qui touchent ce sommet) -> aspect lisse


// compute_face_normals : une normale par TRIANGLE
//  Pour un triangle (v0, v1, v2) :
//    e1 = v1 - v0   et   e2 = v2 - v0 normale = produit vectoriel e1 x e2, normalisé
//
//  Le sens (vers l'extérieur ou l'intérieur de l'objet) dépend de l'ORDRE
//  des sommets du triangle (le "winding").

//  Attention : Normalize() ne fais rien sur vecteur nul, pour plus tard il faut verifier
void compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3_SSE>& out)
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

//  compute_vertex_normals : normales lissées paar sommet
//  Entrée : le mesh + les normales de faces
//  Sortie : "out", un tableau de vertCount normales
//
//  Idée : un sommet est partagé par plusieurs triangles. On additionne
//  les normales de tous ces triangles puis on renormalise : ça donne
//  la "moyenne". Sur une sphère ça donne un rendu lisse, alors que
//  les normales de faces donneraient un rendu "facetté".

// Normales lissées : moyenne des normales des faces qui partagent un sommet
void compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3_SSE>& face_normals, std::vector<Maths::Vec3_SSE>& out)
{
    const int vertCount = mesh.vertCount();
    const int faceCount = mesh.faceCount();

    // 1. Tout remettre à zéro (resize + assign évite les anciennes valeurs)
    out.assign(vertCount, Maths::Vec3_SSE(0, 0, 0));        // remise à zéro

    // 2. Chaque triangle ajoute sa normale à ses 3 sommets
    for (int f = 0; f < faceCount; f++) {                   // accumulation
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