#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/FrameBuffer.h"

int main() {
    const int FB_WIDTH = 320;
    const int FB_HEIGHT = 200;
    const int SCALE = 4;   // fenêtre = 1280 x 800

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CPU Renderer", FB_WIDTH * SCALE, FB_HEIGHT * SCALE, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer)
    {
        std::cerr << "Creation fenetre/renderer: " << SDL_GetError() << "\n";
        return 1;
    }

    // Texture qui reçoit les pixels du framebuffer
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        FB_WIDTH, FB_HEIGHT);

    // Pixels nets (pas de lissage) quand on agrandit
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    FrameBuffer fb(FB_WIDTH, FB_HEIGHT);

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }

        // --- Rendu CPU ---
        fb.Clear(PackColor(30, 30, 60));
        //fb.SetPixel(160, 100, PackColor(255, 0, 0));
        for (int y = 0; y < FB_HEIGHT; y++) {
            for (int x = 0; x < FB_WIDTH; x++) {
                uint8_t r = (uint8_t)(x * 255 / FB_WIDTH);
                uint8_t b = (uint8_t)(y * 255 / FB_HEIGHT);
                fb.SetPixel(x, y, PackColor(r, 64, b));
            }
        }

        // --- Affichage ---
        SDL_UpdateTexture(texture, nullptr, fb.Data(), FB_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}