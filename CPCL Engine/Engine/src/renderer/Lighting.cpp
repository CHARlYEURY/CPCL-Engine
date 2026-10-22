#include "renderer/Lighting.h"

// x^32 par 5 élévations au carré (5 multiplications au lieu d'un std::pow).
// je l'ai mise ici car elle était dans le tuto mais j'utilise std::pow, donc a voire laquelle est la meilleur, pareil on verra pendant le clean 
float pow32(float x)
{
    float x2 = x * x;
    float x4 = x2 * x2;
    float x8 = x4 * x4;
    float x16 = x8 * x8;
    return x16 * x16;
}

// Combine la couleur de texture avec l'éclairage Phong :
// canal = texel * diffuse + blanc * spéculaire
uint32_t color_phong(uint32_t texel, float diffuse_k, float spec_k)
{
    float r = (float)((texel >> 16) & 0xFF) * diffuse_k + 255.0f * spec_k;
    float g = (float)((texel >> 8) & 0xFF) * diffuse_k + 255.0f * spec_k;
    float b = (float)(texel & 0xFF) * diffuse_k + 255.0f * spec_k;
    if (r > 255.0f) r = 255.0f;
    if (g > 255.0f) g = 255.0f;
    if (b > 255.0f) b = 255.0f;
    return ((uint32_t)0xFFu << 24)
        | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

// Multiplie une couleur par k (clampé à [0,1]), je l'utilise plus non plus mais présent dans les tuto donc je la garde pour l'instant
uint32_t color_scale(uint32_t c, float k)
{
    if (k < 0.0f) k = 0.0f;
    if (k > 1.0f) k = 1.0f;
    uint32_t r = (uint32_t)((float)((c >> 16) & 0xFF) * k);
    uint32_t g = (uint32_t)((float)((c >> 8) & 0xFF) * k);
    uint32_t b = (uint32_t)((float)(c & 0xFF) * k);
    return ((uint32_t)0xFFu << 24) | (r << 16) | (g << 8) | b;
}
