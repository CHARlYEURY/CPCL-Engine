#pragma once
#include "Mesh.h"

const int n_points = 32;
const float taille = 10;
const float step = taille - (n_points - 1);

void generate_Terrain(Mesh& mesh);