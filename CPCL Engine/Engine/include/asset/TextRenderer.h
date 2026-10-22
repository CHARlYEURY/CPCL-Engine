#pragma once
#include <cstdint>
#include <string>
#include <SDL3/SDL.h>
#include "asset/Font.h"

// Rend le texte dans une surface ARGB8888 (à libérer avec SDL_DestroySurface)
SDL_Surface * text_make_surface(Font & font, const std::string & text,
    uint16_t size, SDL_Color color);

// Copie la surface dans le framebuffer à (x, y), avec transparence et clipping
void text_blit(SDL_Surface* s, uint32_t* pixels, int fbW, int fbH, int x, int y);
