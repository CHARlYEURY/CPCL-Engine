#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/FrameBuffer.h"
#include "renderer/Rasterizer.h"
#include "Vec3_SSE.h"
#include "Matrix4x4_SSE.h"
#include <numbers>


 const int WIDTH = 800;
 const int HEIGHT = 600;
 const int SCALE = 1;  
 const float PI = std::numbers::pi;


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



static void draw_circle_outline(int cx, int cy, int r, uint32_t color, Rasterizer& rtz, FrameBuffer& fb) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while (x <= y) {
        // the eight symmetric points
         fb.SetPixel(cx + x, cy + y, color);
         fb.SetPixel(cx - x, cy + y, color);
         fb.SetPixel(cx + x, cy - y, color);
         fb.SetPixel(cx - x, cy - y, color);
         fb.SetPixel(cx + y, cy + x, color);
         fb.SetPixel(cx - y, cy + x, color);
         fb.SetPixel(cx + y, cy - x, color);
         fb.SetPixel(cx - y, cy - x, color);
        x++;
        if (d > 0) {
            y--;
            d += 4 * (x - y) + 10;
        }
        else {
            d += 4 * x + 6;
        }
    }
}


















int main() {
    // fenetre = 1280 x 800

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

    // Texture qui reçoit les pixels du framebuffer
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    const Uint64 freq = SDL_GetPerformanceFrequency();
    const Uint64 start = SDL_GetPerformanceCounter();
    Uint64 last = start;
    double fps_accum = 0.0;
    int    fps_count = 0;
    char   title[160];


    // Pixels nets (pas de lissage) quand on agrandit
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    FrameBuffer fb(WIDTH, HEIGHT);
    Rasterizer rtz(fb);




    // An equilateral triangle defined around its own centre (the
    // origin), with corners on a circle of this radius. Three points
    // 120 degrees apart.
    const float R = 200.0f;
    Maths::Vec3_SSE base[3];
    for (int i = 0; i < 3; i++) {
        float a = (float)i / 3.0f * 2.0f * PI - PI / 2.0f;  // start at top
        base[i] = Maths::Vec3_SSE(R * cosf(a), R * sinf(a), 0.0f);
    }

    // One colour per corner: red, green, blue. The rasterizer blends
    // them across the triangle (chapter 3's barycentric gradient).
    const uint32_t corner_color[3] = {
        setRGB(255,  60,  60),   // top
        setRGB(60, 255,  60),   // bottom-right
        setRGB(60,  60, 255),   // bottom-left
    };

    // FPS measurement (chapter 2). From this chapter on, every demo
    // does real per-frame work, so the FPS / ms-per-frame number in
    // the title is meaningful — and the gap between C and Python is
    // dramatic.




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

         




        // Seconds since launch → rotation angle.
        double seconds = (double)(SDL_GetPerformanceCounter() - start) / (double)freq;
        float angle = (float)seconds;   // 1 radian per second

        // Build the transform: rotate around the centre, then move
        // to the middle of the screen. Rightmost matrix is applied
        // first, so this is "rotate, then translate".
        //
        // A positive angle spins the triangle CLOCKWISE here, because
        // the framebuffer is y-down (chapter 1). The rotation matrix
        // is the standard math one (counter-clockwise in y-up); the
        // y-down screen just flips the visual direction. See README
        // section 4.8.
        Maths::Matrix4x4_SSE transform = 
            Maths::Matrix4x4_SSE::Translation(Maths::Vec3_SSE(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f)) *
            Maths::Matrix4x4_SSE::RotationZ(angle);

        // Apply the transform to each corner.
        Maths::Vec3_SSE p[3];
        for (int i = 0; i < 3; i++) p[i] = transform.TransformPoint(base[i]);


        // Reference circle (faint), centred on screen.
        draw_circle_outline(WIDTH / 2, HEIGHT / 2, (int)R, setRGB(60, 60, 80), rtz, fb);

        // Filled triangle with an R/G/B corner gradient.
        draw_triangle((int)p[0].reg.m128_f32[0], (int)p[0].reg.m128_f32[1], corner_color[0],
            (int)p[1].reg.m128_f32[0], (int)p[1].reg.m128_f32[1], corner_color[1],
            (int)p[2].reg.m128_f32[0], (int)p[2].reg.m128_f32[1], corner_color[2], rtz, fb);

        // White outline so the corners on the circle are easy to see.
       rtz.draw_line((int)p[0].reg.m128_f32[0], (int)p[0].reg.m128_f32[1], (int)p[1].reg.m128_f32[0], (int)p[1].reg.m128_f32[1], setRGB(255, 255, 255));
       rtz.draw_line((int)p[1].reg.m128_f32[0], (int)p[1].reg.m128_f32[1], (int)p[2].reg.m128_f32[0], (int)p[2].reg.m128_f32[1], setRGB(255, 255, 255));
       rtz.draw_line((int)p[2].reg.m128_f32[0], (int)p[2].reg.m128_f32[1], (int)p[0].reg.m128_f32[0], (int)p[0].reg.m128_f32[1], setRGB(255, 255, 255));




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