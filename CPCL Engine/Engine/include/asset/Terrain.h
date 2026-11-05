#pragma once
#include "Mesh.h"
#include <iostream>

const int n_points = 32; // nombre de points par côté de la grille (32x32 = 1024 sommets)
const float taille = 10; // longueur d'un côté du terrain, en unités monde
const float step = taille / (n_points - 1); // écart entre deux points voisins

void generate_Terrain(Mesh& mesh);

float hash(int ix, int iy); // Pseudo-aléatoire reproductible : (ix, iy) -> nombre entre 0 et 1


float value_noise(float x, float z); // Bruit lisse 2D : (x, z) -> valeur entre 0 et 1

// Somme de "octaves" bruits de plus en plus fins -> hauteur du relief en (x, z)
float fBm(float x, float z, int octaves = 4);