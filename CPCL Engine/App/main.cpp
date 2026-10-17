#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/FrameBuffer.h"
#include "renderer/Rasterizer.h"
#include "Vec3_SSE.h"
#include "Matrix4x4_SSE.h"
#include <numbers>
#include "asset/AssetLoader.h"
#include "renderer/Z_buffer.h"
#include "asset/textureLoader.h"


 const int WIDTH = 800;
 const int HEIGHT = 600;
 const int SCALE = 1;  
 const float PI = std::numbers::pi;




int main() {
    

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

	Texture tex;
	if (!texture_load("Assets/spot_texture.png", tex)) return 1;
	printf("loaded spot_texture.png: %d x %d\n", tex.width, tex.height);



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
    std::vector<float> inv_w(vertCount);
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
            float w = clip.reg.m128_f32[3];
            if (w <= 0.0001f) { visible[i] = false; continue; }

            float invw = 1.0f / w;
            float ndc_x = clip.reg.m128_f32[0] * invw;
            float ndc_y = clip.reg.m128_f32[1] * invw;
            float nz = clip.reg.m128_f32[2] * invw;

            screen_x[i] = (int)((ndc_x + 1.0f) * 0.5f * WIDTH);
            screen_y[i] = (int)((1.0f - (ndc_y + 1.0f) * 0.5f) * HEIGHT);
            ndc_z[i] = nz;
            inv_w[i] = invw;          // <-- la ligne qui manquait
            visible[i] = true;
        }
        float zrange = (zmax > zmin) ? (zmax - zmin) : 1.0f;

        fb.Clear(setRGB(16, 16, 24));
        zb.clear_Zbuffer();

        int drawn = 0, culled = 0;

        // Fill every triangle. Order no longer matters: the depth
        // test, not the draw order, decides what ends up visible.
        for (int fidx = 0; fidx < mesh.faceCount(); fidx++) {
            int a = mesh.faces[fidx * 3 + 0];
            int b = mesh.faces[fidx * 3 + 1];
            int c = mesh.faces[fidx * 3 + 2];
            if (!visible[a] || !visible[b] || !visible[c]) continue;

            int area = zb.edge(screen_x[a], screen_y[a], screen_x[b], screen_y[b], screen_x[c], screen_y[c]);

            if (area >= 0) { culled++; continue; }
            int ua = mesh.uvFaces[fidx * 3 + 0];
            int ub = mesh.uvFaces[fidx * 3 + 1];
            int uc = mesh.uvFaces[fidx * 3 + 2];
            UV uva = (ua >= 0) ? mesh.uvs[ua] : UV(0, 0);
            UV uvb = (ub >= 0) ? mesh.uvs[ub] : UV(0, 0);
            UV uvc = (uc >= 0) ? mesh.uvs[uc] : UV(0, 0);

            zb.draw_triangle_zbuffer(screen_x[a], screen_y[a], ndc_z[a], inv_w[a], uva,
                screen_x[b], screen_y[b], ndc_z[b], inv_w[b], uvb,
                screen_x[c], screen_y[c], ndc_z[c], inv_w[c], uvc,
                rtz ,fb ,tex);
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
                "Chapter 08 — Backface culling  |  drew %d, culled %d of %d  |  %.1f FPS  (%.2f ms)",
                drawn, culled, mesh.faceCount(), fps, ms);
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