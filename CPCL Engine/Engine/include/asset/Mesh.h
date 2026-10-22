#pragma once
#include "Vec3_SSE.h"
#include <vector>

struct Mesh {
    std::vector<Maths::Vec3_SSE> verts;
    std::vector<int> faces;    // 3 ints per face: v0, v1, v2 (0-based)
    std::vector<int> uvFaces;    // 3 ints per face: v0, v1, v2 (0-based)
	std::vector<Maths::UV> uvs;        // 1 UV per vertex
    int vertCount() const { return (int)verts.size(); }
    int faceCount() const { return (int)faces.size() / 3; }
	int uvCount() const { return (int)uvs.size(); }
};

// Une normale unitaire par triangle : cross(v1-v0, v2-v0) normalisé
// Le sens (vers l'extérieur ou l'intérieur) dépend du winding du mesh
void compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3_SSE>& out);

// Normales lissées : moyenne des normales des faces qui partagent un sommet
void compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3_SSE>& face_normals, std::vector<Maths::Vec3_SSE>& out);
