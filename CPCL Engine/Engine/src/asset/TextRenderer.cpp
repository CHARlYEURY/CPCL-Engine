#include "asset/TextRenderer.h"
#include <iostream>

SDL_Surface* text_make_surface(Font& font, const std::string& text,
    uint16_t size, SDL_Color color) {
    TTF_Font* f = font.getFont(size);
    if (!f) return nullptr;

    SDL_Surface* raw = TTF_RenderText_Blended(f, text.c_str(), 0, color);
    if (!raw) {
        std::cerr << "TTF_RenderText_Blended: " << SDL_GetError() << "\n";
        return nullptr;
    }
    // On force le même format que le framebuffer
    SDL_Surface* s = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_ARGB8888);
    SDL_DestroySurface(raw);
    return s;
}

void text_blit(SDL_Surface* s, uint32_t* pixels, int fbW, int fbH, int x0, int y0) {
    if (!s || !pixels) return;
    if (!SDL_LockSurface(s)) return;

    for (int y = 0; y < s->h; y++) {
        int dy = y0 + y;
        if (dy < 0 || dy >= fbH) continue;
        const uint32_t* src = (const uint32_t*)((const uint8_t*)s->pixels + y * s->pitch);

        for (int x = 0; x < s->w; x++) {
            int dx = x0 + x;
            if (dx < 0 || dx >= fbW) continue;

            uint32_t p = src[x];
            uint32_t a = p >> 24;
            if (a == 0) continue;

            uint32_t& d = pixels[dy * fbW + dx];
            if (a == 255) { d = p; continue; }

            uint32_t ia = 255 - a;
            uint32_t r = (((p >> 16) & 255) * a + ((d >> 16) & 255) * ia) / 255;
            uint32_t g = (((p >> 8) & 255) * a + ((d >> 8) & 255) * ia) / 255;
            uint32_t b = ((p & 255) * a + (d & 255) * ia) / 255;
            d = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
    SDL_UnlockSurface(s);
}
