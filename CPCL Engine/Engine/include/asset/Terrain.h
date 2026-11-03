#pragma once
#include "Mesh.h"
#include <iostream>

const int n_points = 32;
const float taille = 10;
const float step = taille / (n_points - 1);

void generate_Terrain(Mesh& mesh);

float hash(int ix, int iy);

float value_noise(float x, float z);

float fBm(float x, float z, int octaves = 4);