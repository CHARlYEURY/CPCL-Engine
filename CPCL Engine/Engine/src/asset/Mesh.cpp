#include "asset/Mesh.h"

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

// Une normale unitaire par triangle : cross(v1-v0, v2-v0) normalisé
// Le sens (vers l'extérieur ou l'intérieur) dépend du winding du mesh
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
