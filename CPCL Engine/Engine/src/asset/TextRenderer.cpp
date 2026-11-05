#include "asset/TextRenderer.h"
#include <iostream>

//  Affiche du texte DANS notre framebuffer
//  Deux étapes :
//    1) text_make_surface : SDL_ttf dessine le texte dans une image (SDL_Surface)
//    2) text_blit         : on copie cette image pixel par pixel dans le framebuffer, en gérant la transparence.


//  text_make_surface : transforme un texte en image (SDL_Surface)
//    font  : notre classe Font (donne le TTF_Font à la bonne taille)
//    text  : le texte à écrire
//    size  : taille de la police
//    color : couleur du texte 
SDL_Surface* text_make_surface(Font& font, const std::string& text,
    uint16_t size, SDL_Color color) {
    TTF_Font* f = font.getFont(size);
    if (!f) return nullptr;

    // "Blended" = texte anti-aliasé (bords doux grâce à l'alpha)
	// le 0 = la chaîne se termine par \0, pas de precision sur la longueur
    SDL_Surface* raw = TTF_RenderText_Blended(f, text.c_str(), 0, color); 
    if (!raw) {
        std::cerr << "TTF_RenderText_Blended: " << SDL_GetError() << "\n";
        return nullptr;
    }
    // On force le même format que le framebuffer (ARGB8888 = 0xAARRGGBB)
    SDL_Surface* s = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_ARGB8888);
    SDL_DestroySurface(raw); // la surface d'origine ne sert plus
    return s;
}

//    pixels   : fb.Data()
//    "Blit" = copie d'un bloc de pixels d'une image vers une autre.
// 
//  Pour chaque pixel du texte :
//    - hors de l'écran -> on l'ignore "clipping"
//    - alpha = 0       -> transparent, on ne touche pas au framebuffer
//    - alpha = 255     -> opaque, on remplace le pixel
//    - entre les deux  -> on mélange (bords anti-aliasés) :
//          résultat = texte * a + fond * (1 - a)     (a entre 0 et 1)
void text_blit(SDL_Surface* s, uint32_t* pixels, int fbW, int fbH, int x0, int y0) {  // colle la surface dans le framebuffer à la position (x0, y0)
    if (!s || !pixels) return;
    if (!SDL_LockSurface(s)) return;       // on verrouille la surface pour avoir le droit de lire ses pixels

    for (int y = 0; y < s->h; y++) {
        int dy = y0 + y;                   // ligne correspondante dans le framebuffer
        if (dy < 0 || dy >= fbH) continue; // ligne hors écran
        // début de la ligne y de la surface ; pitch = taille d'une ligne EN OCTETS
        // (cast en uint8_t* pour avancer en octets, puis retour en uint32_t*)
        const uint32_t* src = (const uint32_t*)((const uint8_t*)s->pixels + y * s->pitch);

        for (int x = 0; x < s->w; x++) {
            int dx = x0 + x;
            if (dx < 0 || dx >= fbW) continue;

			uint32_t p = src[x]; // pixel du texte (ARGB8888)
            uint32_t a = p >> 24;
            if (a == 0) continue;

            uint32_t& d = pixels[dy * fbW + dx];
            if (a == 255) { d = p; continue; }

            // mélange canal par canal (r, g, b) ; ia = part du fond qui reste visible
            uint32_t ia = 255 - a;
            uint32_t r = (((p >> 16) & 255) * a + ((d >> 16) & 255) * ia) / 255;
            uint32_t g = (((p >> 8) & 255) * a + ((d >> 8) & 255) * ia) / 255;
            uint32_t b = ((p & 255) * a + (d & 255) * ia) / 255;
            d = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
    SDL_UnlockSurface(s);
}
