#pragma once
#include "asset/Mesh.h"
#include "asset/textureLoader.h"
#include "asset/Materials.h"


struct MeshRenderer {
    const Mesh* mesh;
    const Materials* materials;
    const std::vector<Maths::Vec3_SSE>* vertexNormals;
};
