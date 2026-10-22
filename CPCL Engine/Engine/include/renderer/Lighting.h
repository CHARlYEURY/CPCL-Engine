#pragma once
#include <cstdint>

#define AMBIENT 0.18f

// x^32 par 5 élévations au carré (5 multiplications au lieu d'un std::pow).
float pow32(float x);

// Combine la couleur de texture avec l'éclairage Phong :
// canal = texel * diffuse + blanc * spéculaire
uint32_t color_phong(uint32_t texel, float diffuse_k, float spec_k);

// Multiplie une couleur par k (clampé à [0,1])
uint32_t color_scale(uint32_t c, float k);
