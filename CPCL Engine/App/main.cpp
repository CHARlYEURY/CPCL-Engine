#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/FrameBuffer.h"
#include "renderer/Rasterizer.h"


 const int WIDTH = 800;
 const int HEIGHT = 600;
 const int SCALE = 1;  

// Bresenham's line, generalised to all 8 octants.
// See the chapter README, step 5, for what each line is doing.
//
// All arithmetic is integer. No multiplication or division inside
// the loop - only adds and a sign test.
//static void draw_line(int x0, int y0, int x1, int y1, uint32_t color, FrameBuffer& fb) {
//    int dx = abs(x1 - x0);
//    int dy = -abs(y1 - y0);             // negative on purpose
//    int sx = (x0 < x1) ? 1 : -1;
//    int sy = (y0 < y1) ? 1 : -1;
//    int err = dx + dy;                   // = dx - |dy|0
//
//    for (;;) {
//        fb.SetPixel(x0, y0, color);
//        if (x0 == x1 && y0 == y1) break;
//        int e2 = 2 * err;
//        if (e2 >= dy) { err += dy; x0 += sx; }   // step in x
//        if (e2 <= dx) { err += dx; y0 += sy; }   // step in y
//    }
//}


static void draw_triangle(int x0, int y0, uint32_t c0,
    int x1, int y1, uint32_t c1,
    int x2, int y2, uint32_t c2, Rasterizer& rtz, FrameBuffer& fb) {
    int area = rtz.edge(x0, y0, x1, y1, x2, y2);
    if (area == 0) return;                 // degenerate sliver

    // Force one winding so "inside" means "all weights >= 0".
    if (area < 0) {
        int tx = x1; int ty = y1; uint32_t tc = c1;
        x1 = x2; y1 = y2; c1 = c2;
        x2 = tx; y2 = ty; c2 = tc;
        area = -area;
    }

    // Bounding box, clamped to the screen.
    int minx = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int miny = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int maxx = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int maxy = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx >= WIDTH)  maxx = WIDTH - 1;
    if (maxy >= HEIGHT) maxy = HEIGHT - 1;

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            int e0 = rtz.edge(x1, y1, x2, y2, x, y);
            int e1 = rtz.edge(x2, y2, x0, y0, x, y);
            int e2 = rtz.edge(x0, y0, x1, y1, x, y);
            // Inside iff all three weights are non-negative.
            if ((e0 | e1 | e2) >= 0) {
                fb.SetPixel(x, y, rtz.shade(e0, e1, e2, area, c0, c1, c2));
            }
        }
    }
}





int main() {
    // fenêtre = 1280 x 800

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CPU Renderer", WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_TRANSPARENT);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!window || !renderer)
    {
        std::cerr << "Creation fenetre/renderer: " << SDL_GetError() << "\n";
        return 1;
    }

    const Uint64 freq = SDL_GetPerformanceFrequency();
    Uint64 last = SDL_GetPerformanceCounter();
    double fps_accum = 0.0;
    int    fps_count = 0;
    char   title[160];



    // Texture qui reçoit les pixels du framebuffer
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    // Pixels nets (pas de lissage) quand on agrandit
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    FrameBuffer fb(WIDTH, HEIGHT);
    Rasterizer rtz(fb);





    /////////////////////// run /////////////////////

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
        fb.Clear(setRGB(0, 0, 0, 0));

        draw_triangle(150, 480, setRGB(255, 0, 0),
            400, 80, setRGB(0, 255, 0),
            650, 480, setRGB(0, 0, 255), rtz, fb);

        // 2) Small flat-shaded triangle (same colour x3).
        draw_triangle(60, 60, setRGB(240, 240, 240),
            200, 60, setRGB(240, 240, 240),
            130, 170, setRGB(240, 240, 240),rtz, fb);

        // 3) White wireframe over the big triangle's edges.
        rtz.draw_line(150, 480, 400, 80, setRGB(255, 255, 255));
        rtz.draw_line(400, 80, 650, 480, setRGB(255, 255, 255));
        rtz.draw_line(650, 480, 150, 480, setRGB(255, 255, 255));

        // --- Affichage ---
        SDL_UpdateTexture(texture, nullptr, fb.Data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // ---- Update the FPS counter ----
        Uint64 now = SDL_GetPerformanceCounter();
        double dt = (double)(now - last) / (double)freq;
        last = now;
        fps_accum += dt;
        fps_count += 1;
        if (fps_accum >= 1.0) {
            double fps = (double)fps_count / fps_accum;
            double ms = 1000.0 * fps_accum / (double)fps_count;
            snprintf(title, sizeof title,
                "Chapter 02 — Lines  |  %.1f FPS  (%.2f ms/frame)",
                fps, ms);
            SDL_SetWindowTitle(window, title);
            fps_accum = 0.0;
            fps_count = 0;

        }

    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}