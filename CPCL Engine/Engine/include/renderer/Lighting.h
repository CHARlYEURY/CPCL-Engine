// ============================================================
//  Lighting.h
// ------------------------------------------------------------
//  petites fonctions utilitaires pour l'eclairage et les couleurs.
//  le calcul des angles (lambert / blinn-phong) est fait dans le rasterizer,
//  ici on a seulement ce qui sert a COMBINER les resultats en une couleur finale :
//     - color_phong   : texture + diffus + speculaire -> couleur du pixel
//     - blend_colors  : melange avec le fond pour la transparence
//     - pow32 / color_scale : outils gardés du tuto, pas utilisés pour l'instant
//
//  les couleurs sont toutes au format 0xAARRGGBB (voir Framebuffer.h).
//  utilisé par : Rasterizer::draw_triangle_phong
// ============================================================
#pragma once
#include <cstdint>
#include "renderer/Framebuffer.h"   

// lumiere ambiante : eclairage minimum que recoit TOUT objet, meme dans l'ombre
// (0.18 = 18% de la couleur de base, sinon les faces opposées a la lumiere seraient noires)
// utilisée dans le rasterizer : diffuse_k = AMBIENT + (1 - AMBIENT) * lambert
#define AMBIENT 0.18f

// x^32 par 5 élévations au carré (5 multiplications au lieu d'un std::pow).
// sert pour la brillance du speculaire (exposant 32)
float pow32(float x);

// Combine la couleur de texture avec l'éclairage Phong :
// canal = texel * diffuse + blanc * spéculaire
// diffuse_k = intensité de la lumiere diffuse (ambiante comprise), spec_k = intensité du reflet blanc
uint32_t color_phong(uint32_t texel, float diffuse_k, float spec_k);

// Multiplie une couleur par k (clampé à [0,1])
uint32_t color_scale(uint32_t c, float k);

// Mélange la couleur source et destination selon l'alpha (0..255)
// src = couleur qu'on dessine, dst = couleur deja dans le framebuffer
// alpha = 255 -> src seulement, alpha = 0 -> dst seulement
uint32_t blend_colors(uint32_t src, uint32_t dst, uint8_t alpha);