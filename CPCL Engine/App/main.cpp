#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/FrameBuffer.h"
#include "renderer/Rasterizer.h"
#include "Vec3_SSE.h"
#include "Matrix4x4_SSE.h"
#include <numbers>
#include "asset/AssetLoader.h"
#include "renderer/Z_buffer.h"


 const int WIDTH = 800;
 const int HEIGHT = 600;
 const int SCALE = 1;  
 const float PI = std::numbers::pi;

#define CULL_BACKFACES true

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
//
//
//
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
	ZBuffer zb(WIDTH, HEIGHT);






    Mesh mesh;
    if (!mesh_load("Assets/spot.obj", mesh)) return 1;
    printf("loaded spot.obj: %d vertices, %d faces\n",
        mesh.vertCount(), mesh.faceCount());





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
    const auto eye = Maths::Vec3_SSE(0, 0, 3);
    const auto target = Maths::Vec3_SSE(0, 0, 0);
    const auto  up = Maths::Vec3_SSE(0, 1, 0);
    const auto view = Maths::Matrix4x4_SSE::LookAt(eye, target, up);
    ////////////////////// CAMERA ////////////////////////////



    const int vertCount = mesh.vertCount();
    std::vector<int>   screen_x(vertCount);
    std::vector<int>   screen_y(vertCount);
    std::vector<float>   ndc_z(vertCount);
    std::vector<float> cam_depth(vertCount);
    std::vector<char>  visible(vertCount);

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
        Maths::Vec4_SSE vert;

       


        // Project every vertex once. We store NDC z (clip.z / clip.w):
        // that is the value the Z-buffer compares, and it interpolates
        // correctly in screen space. We also track its min/max across
        // the model so the depth shading uses the full grey range
        // (NDC z barely changes across a small object, so without this
        // rescale the whole cow would be one flat shade).
        float zmin = 1e30f, zmax = -1e30f;
		Maths::Vec4_SSE point;
        for (int i = 0; i < mesh.vertCount(); i++) {
            Maths::Vec4_SSE clip = mvp * point.FromPoint(mesh.verts[i]);
            if (clip.reg.m128_f32[3] <= 0.0001f) { visible[i] = false; continue; }
            float ndc_x = clip.reg.m128_f32[0] / clip.reg.m128_f32[3];
            float ndc_y = clip.reg.m128_f32[1] / clip.reg.m128_f32[3];
            float nz = clip.reg.m128_f32[2] / clip.reg.m128_f32[3];
            screen_x[i] = (int)((ndc_x + 1.0f) * 0.5f * WIDTH);
            screen_y[i] = (int)(((1.0f - ndc_y) * 0.5f) * HEIGHT);
            ndc_z[i] = nz;
            visible[i] = true;
            if (nz < zmin) zmin = nz;
            if (nz > zmax) zmax = nz;
        }
        float zrange = (zmax > zmin) ? (zmax - zmin) : 1.0f;

        fb.Clear(setRGB(16, 16, 24));
        zb.clear_Zbuffer();

        // Fill every triangle. Order no longer matters: the depth
        // test, not the draw order, decides what ends up visible.
        int drawn = 0, culled = 0;
        for (int fidx = 0; fidx < mesh.faceCount(); fidx++) {
            int a = mesh.faces[fidx * 3 + 0];
            int b = mesh.faces[fidx * 3 + 1];
            int c = mesh.faces[fidx * 3 + 2];
            if (!visible[a] || !visible[b] || !visible[c]) continue;


            int area = rtz.edge(screen_x[a], screen_y[a], screen_x[b], screen_y[b], screen_x[c], screen_y[c]);

#if CULL_BACKFACES
            if (area >= 0) { culled++; continue; }
#endif
            (void)area;
            drawn++;


            zb.draw_triangle_zbuffer(screen_x[a], screen_y[a], ndc_z[a],
                screen_x[b], screen_y[b], ndc_z[b],
                screen_x[c], screen_y[c], ndc_z[c],
                zmin, zrange, rtz, fb);
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