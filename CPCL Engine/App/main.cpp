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


//static void draw_triangle(int x0, int y0, uint32_t c0,
//    int x1, int y1, uint32_t c1,
//    int x2, int y2, uint32_t c2, Rasterizer& rtz, FrameBuffer& fb) {
//    int area = rtz.edge(x0, y0, x1, y1, x2, y2);
//    if (area == 0) return;                 // degenerate sliver
//
//    // Force one winding so "inside" means "all weights >= 0".
//    if (area < 0) {
//        int tx = x1; int ty = y1; uint32_t tc = c1;
//        x1 = x2; y1 = y2; c1 = c2;
//        x2 = tx; y2 = ty; c2 = tc;
//        area = -area;
//    }
//
//    // Bounding box, clamped to the screen.
//    int minx = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
//    int miny = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
//    int maxx = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
//    int maxy = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);
//    if (minx < 0) minx = 0;
//    if (miny < 0) miny = 0;
//    if (maxx >= WIDTH)  maxx = WIDTH - 1;
//    if (maxy >= HEIGHT) maxy = HEIGHT - 1;
//
//    for (int y = miny; y <= maxy; y++) {
//        for (int x = minx; x <= maxx; x++) {
//            int e0 = rtz.edge(x1, y1, x2, y2, x, y);
//            int e1 = rtz.edge(x2, y2, x0, y0, x, y);
//            int e2 = rtz.edge(x0, y0, x1, y1, x, y);
//            // Inside iff all three weights are non-negative.
//            if ((e0 | e1 | e2) >= 0) {
//                fb.SetPixel(x, y, rtz.shade(e0, e1, e2, area, c0, c1, c2));
//            }
//        }
//    }
//}



//static void draw_circle_outline(int cx, int cy, int r, uint32_t color, Rasterizer& rtz, FrameBuffer& fb) {
//    int x = 0;
//    int y = r;
//    int d = 3 - 2 * r;
//    while (x <= y) {
//        // the eight symmetric points
//         fb.SetPixel(cx + x, cy + y, color);
//         fb.SetPixel(cx - x, cy + y, color);
//         fb.SetPixel(cx + x, cy - y, color);
//         fb.SetPixel(cx - x, cy - y, color);
//         fb.SetPixel(cx + y, cy + x, color);
//         fb.SetPixel(cx - y, cy + x, color);
//         fb.SetPixel(cx + y, cy - x, color);
//         fb.SetPixel(cx - y, cy - x, color);
//        x++;
//        if (d > 0) {
//            y--;
//            d += 4 * (x - y) + 10;
//        }
//        else {
//            d += 4 * x + 6;
//        }
//    }
//}






 // The 8 corners of a unit cube, centred on the origin (each
// coordinate is +1 or -1).
 static const Maths::Vec3_SSE CUBE_VERTS[8] = {
     {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
     {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1},
 };

 // The 12 edges, as pairs of corner indices, plus an axis tag
 // (0 = runs along x, 1 = along y, 2 = along z) used to colour them.
 static const int CUBE_EDGES[12][3] = {
     {0,1,0}, {3,2,0}, {7,6,0}, {4,5,0},   // edges along x
     {0,3,1}, {1,2,1}, {5,6,1}, {4,7,1},   // edges along y
     {0,4,2}, {1,5,2}, {2,6,2}, {3,7,2},   // edges along z
 };











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




    // Edge colour per axis (rgb() isn't a compile-time constant, so
       // we fill this at runtime rather than as a static initializer).
    uint32_t axis_color[3];
    axis_color[0] = setRGB(230, 80, 80);   // x : red
    axis_color[1] = setRGB(80, 210, 90);   // y : green
    axis_color[2] = setRGB(90, 140, 240);   // z : blue




    ////////////////////// CAMERA ////////////////////////////
    // Projection and view stay fixed; only the model spins.
    const float aspect = (float)WIDTH / (float)HEIGHT;
    const float fovY = 60.0f * PI / 180.0f;
    // 60-degree vertical field of view (radians() converts to the
    // radians the math needs); window shape; near and far clip
    // distances. See README section 5.6.
    const Maths::Matrix4x4_SSE projection = Maths::Matrix4x4_SSE::Perspective(fovY, aspect,
        0.1f, 100.0f);
    // Camera: positioned 5 units back on +z (eye), aimed at the
    // origin (target), with world-up being +y. See README section 5.10.
    const auto eye = Maths::Vec3_SSE(0, 0, 5);
    const auto target = Maths::Vec3_SSE(0, 0, 0);
    const auto  up = Maths::Vec3_SSE(0, 1, 0);
    const auto view = Maths::Matrix4x4_SSE::LookAt(eye, target, up);
    ////////////////////// CAMERA ////////////////////////////





    /////////////////////// RUN /////////////////////

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

        double seconds = (double)(SDL_GetPerformanceCounter() - start) / (double)freq;
        float t = (float)seconds;



        // Build the model matrix: spin around y, and a bit around x,
        // so we see the cube from a lively angle. Then chain the three
        // matrices into one (reads right-to-left: model, then view,
        // then projection — README section 5.11).
        Maths::Matrix4x4_SSE model = Maths::Matrix4x4_SSE::RotationY(t * 0.8f) *  Maths::Matrix4x4_SSE::RotationX(t * 0.5f);
        Maths::Matrix4x4_SSE mvp = projection * (view * model);

        // Run all 8 cube corners through the pipeline, storing the
        // resulting screen pixel of each. `visible[i]` says whether
        // corner i ended up in front of the camera.
        int  screen_x[8], screen_y[8];
        bool visible[8];
        for (int i = 0; i < 8; i++) {

            Maths::Vec4_SSE vert;
            
            // Step 1 — one matrix multiply (the vertex carries w = 1).
            Maths::Vec4_SSE clip = mvp * vert.FromPoint(CUBE_VERTS[i]);

            // Behind the camera? w came out <= 0; dividing would give
            // garbage, so skip this corner (proper clipping is ch. 12).
            if (clip.reg.m128_f32[3] <= 0.0001f) { visible[i] = false; continue; }

            // Step 2 — the perspective divide. This is where distant
            // things actually get smaller: x and y are divided by the
            // depth that the projection matrix parked in w.
            float ndc_x = clip.reg.m128_f32[0] / clip.reg.m128_f32[3];     // now in -1..+1
            float ndc_y = clip.reg.m128_f32[1] / clip.reg.m128_f32[3];

            // Step 3 — viewport remap: NDC (-1..+1) to pixels. The y
            // term flips top-for-bottom because screen rows count down
            // (chapter 1), while NDC has +y up.
            screen_x[i] = (int)((ndc_x + 1.0f) * 0.5f * WIDTH);
            screen_y[i] = (int)((1.0f - (ndc_y + 1.0f) * 0.5f) * HEIGHT);
            visible[i] = true;
        }

        fb.Clear(setRGB(16, 16, 24));

        // Draw the 12 edges, each connecting two of the projected
        // corners. v0/v1 are the corner indices; axis (0/1/2) picks
        // the edge's colour so x/y/z edges read as red/green/blue.
        for (int i = 0; i < 12; i++) {
            int v0 = CUBE_EDGES[i][0];
            int v1 = CUBE_EDGES[i][1];
            int axis = CUBE_EDGES[i][2];
            if (!visible[v0] || !visible[v1]) continue;
            rtz.draw_line(screen_x[v0], screen_y[v0],
                screen_x[v1], screen_y[v1], axis_color[axis]);
        }



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