// ============================================================
//  Lighting.cpp
// ------------------------------------------------------------
//  implementation des fonctions de couleur / eclairage (voir Lighting.h).
//  rappel du format : 0xAARRGGBB -> on extrait un canal avec (c >> decalage) & 0xFF
//  avec decalage = 16 pour le rouge, 8 pour le vert, 0 pour le bleu.
// ============================================================
#include "renderer/Lighting.h"

// ------------------------------------------------------------
//  pow32 : calcule x puissance 32 sans std::pow
//  Idée : x^32 = (x^16)^2 = ((x^8)^2)^2 ... donc on eleve au carré 5 fois de suite
//  x -> x^2 -> x^4 -> x^8 -> x^16 -> x^32  (5 multiplications)
//  x^32 par 5 élévations au carré (5 multiplications au lieu d'un std::pow).
//  je l'ai mise ici car elle était dans le tuto mais j'utilise std::pow, donc a voire laquelle est la meilleur, pareil on verra pendant le clean 
// ------------------------------------------------------------
float pow32(float x)
{
    float x2 = x * x;          // x^2
    float x4 = x2 * x2;        // x^4
    float x8 = x4 * x4;        // x^8
    float x16 = x8 * x8;       // x^16
    return x16 * x16;          // x^32
}

// ------------------------------------------------------------
//  color_phong : calcule la couleur finale d'un pixel eclairé
//  Combine la couleur de texture avec l'éclairage Phong :
//  canal = texel * diffuse + blanc * spéculaire
//  - la partie diffuse ASSOMBRIT la couleur de la texture (diffuse_k entre AMBIENT et 1)
//  - la partie speculaire AJOUTE du blanc (le reflet brillant n'a pas la couleur de l'objet)
//  la somme peut depasser 255 donc on clampe, sinon le canal "reboucle" au moment du cast en entier
//  l'alpha de sortie est toujours 255 : la transparence est gerée a part (blend_colors)
//  appelée par Rasterizer::draw_triangle_phong, une fois par pixel
// ------------------------------------------------------------
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

// ------------------------------------------------------------
//  color_scale : multiplie une couleur par un facteur k (assombrir / eclaircir)
//  Multiplie une couleur par k (clampé à [0,1]), je l'utilise plus non plus mais présent dans les tuto donc je la garde pour l'instant
//  k est clampé entre 0 et 1 donc on peut seulement assombrir, jamais depasser la couleur d'origine
//  alpha remis a 255. c'etait l'ancienne facon d'eclairer (avant color_phong)
// ------------------------------------------------------------
uint32_t color_scale(uint32_t c, float k)
{
    if (k < 0.0f) k = 0.0f;
    if (k > 1.0f) k = 1.0f;
    uint32_t r = (uint32_t)((float)((c >> 16) & 0xFF) * k);
    uint32_t g = (uint32_t)((float)((c >> 8) & 0xFF) * k);
    uint32_t b = (uint32_t)((float)(c & 0xFF) * k);
    return ((uint32_t)0xFFu << 24) | (r << 16) | (g << 8) | b;
}

// ------------------------------------------------------------
//  blend_colors : melange "alpha blending" classique
//  resultat = src * a + dst * (1 - a)    avec a = alpha / 255 (entre 0 et 1)
//  ex : alpha = 128 -> moitié de la couleur dessinée, moitié de ce qu'il y avait deja derriere
//  le resultat est opaque (setRGB met l'alpha a 255) vu qu'on ecrit dans le framebuffer final
//  appelée par Rasterizer::draw_triangle_phong pour les materiaux transparents
// ------------------------------------------------------------
uint32_t blend_colors(uint32_t src, uint32_t dst, uint8_t alpha)
{
    float a = alpha / 255.0f;       // alpha en pourcentage
    float ia = 1.0f - a;            // poids de ce qu'il y a deja dans le framebuffer

    uint8_t r = (uint8_t)(((src >> 16) & 0xFF) * a + ((dst >> 16) & 0xFF) * ia);
    uint8_t g = (uint8_t)(((src >> 8) & 0xFF) * a + ((dst >> 8) & 0xFF) * ia);
    uint8_t b = (uint8_t)((src & 0xFF) * a + (dst & 0xFF) * ia);

    return setRGB(r, g, b);
}