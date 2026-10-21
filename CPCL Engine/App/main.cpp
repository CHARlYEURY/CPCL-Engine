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
#include "renderer/clip.h"


 const int WIDTH = 800;
 const int HEIGHT = 600;
 const int SCALE = 1;  
 const float PI = std::numbers::pi;

#define AMBIENT 0.18f

 
  


int main() {
    
	/////////////////////////////////////////////////////////// pas faire gaffe ////////////////////////////////////////////////////////////
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

    /////////////////////////////////////////////////////////// pas faire gaffe ////////////////////////////////////////////////////////////

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

    std::vector<Maths::Vec3_SSE> face_normals;
    std::vector<Maths::Vec3_SSE> vertex_normals;

    // Normales calculées UNE fois, en espace objet : le mesh est rigide, elles ne
    // changent pas. Seule la rotation du modèle les fera tourner (plus bas)
    zb.compute_face_normals(mesh, face_normals);
    zb.compute_vertex_normals(mesh, face_normals, vertex_normals);


    // Éclairage en espace MONDE. Ces vecteurs pointent VERS la lumière / la caméra
    const Maths::Vec3_SSE light = Maths::Vec3_SSE(0.4f, 0.8f, 0.45f).Normalize();
    const Maths::Vec3_SSE view_dir(0, 0, 1);
    const Maths::Vec3_SSE half_vec = (light + view_dir).Normalize();


    ////////////////////// CAMERA ////////////////////////////
    const float aspect = (float)WIDTH / (float)HEIGHT;
    const float fovY = 60.0f * PI / 180.0f;
    
    const Maths::Matrix4x4_SSE projection = Maths::Matrix4x4_SSE::Perspective(fovY, aspect,
        0.1f, 100.0f);
   
    const auto eye = Maths::Vec3_SSE(0, 0, 0.2);
    const auto target = Maths::Vec3_SSE(0, 0, 0);
    const auto  up = Maths::Vec3_SSE(0, 1, 0);
    const auto view = Maths::Matrix4x4_SSE::LookAt(eye, target, up);
    ////////////////////// CAMERA ////////////////////////////

   
	
    // Données par sommet, allouées une fois
    
    const int vertCount = mesh.vertCount();
    std::vector<int>   screen_x(vertCount);
    std::vector<int>   screen_y(vertCount);
    std::vector<float>   ndc_z(vertCount);  // profondeur pour le Z-test
    std::vector<float> inv_w(vertCount);    // 1/w pour la correction de perspective
    std::vector<Maths::Vec3_SSE> normal_world(vertCount);
    std::vector<char>  visible(vertCount);
    std::vector<Maths::Vec4_SSE> clip_v(vertCount);


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

       
        double seconds = (double)(SDL_GetPerformanceCounter() - start) / (double)freq;
        float t = (float)seconds;

        

        // Matrice modèle : se lit de droite à gauche → d'abord RotationX, puis RotationY
        Maths::Matrix4x4_SSE model = Maths::Matrix4x4_SSE::RotationY(t * 0.8f) *  Maths::Matrix4x4_SSE::RotationX(t * 0.5f);
        Maths::Matrix4x4_SSE mvp = projection * (view * model);
        Maths::Vec4_SSE vert;
        Maths::Vec4_SSE clip;
        
       for (int i = 0; i < mesh.vertCount(); i++) {
            clip_v[i] = mvp * clip.FromPoint(mesh.verts[i]);
            normal_world[i] = model.TransformDirection(vertex_normals[i]);
        }


        // Project every vertex once. We store NDC z (clip.z / clip.w):
        // that is the value the Z-buffer compares, and it interpolates
        // correctly in screen space. We also track its min/max across
        // the model so the depth shading uses the full grey range
        // (NDC z barely changes across a small object, so without this
        // rescale the whole cow would be one flat shade).
        //float zmin = 1e30f, zmax = -1e30f;
		Maths::Vec4_SSE point;
        for (int i = 0; i < mesh.vertCount(); i++) {
            // Position objet -> clip FromPoint met w = 1 (c'est un point, la translation compte).
            Maths::Vec4_SSE clip = mvp * point.FromPoint(mesh.verts[i]);
            float w = clip.reg.m128_f32[3];
            if (w <= 0.0001f) { visible[i] = false; continue; }

            float invw = 1.0f / w;
            float ndc_x = clip.reg.m128_f32[0] * invw;
            float ndc_y = clip.reg.m128_f32[1] * invw;
            float nz = clip.reg.m128_f32[2] * invw;

            // NDC → pixels. y inversé : en NDC y monte, à l'écran il descend.
            screen_x[i] = (int)((ndc_x + 1.0f) * 0.5f * WIDTH);
            screen_y[i] = (int)((1.0f - (ndc_y + 1.0f) * 0.5f) * HEIGHT);
            ndc_z[i] = nz;
            inv_w[i] = invw;          
			normal_world[i] = model.TransformDirection(vertex_normals[i]);
            visible[i] = true;
        }

        fb.Clear(setRGB(16, 16, 24));
        zb.clear_Zbuffer();           // profondeur remise à « infini »

		int culled = 0;

        Maths::Vec4_SSE point2;
        for (int fidx = 0; fidx < mesh.faceCount(); fidx++) {
            clip_vertex tri[3];
            for (int k = 0; k < 3; k++) {
                int vi = mesh.faces[fidx * 3 + k];
                int ti = mesh.uvFaces[fidx * 3 + k];
                tri[k].clip = mvp * point2.FromPoint(mesh.verts[vi]);
                tri[k].uv = (ti >= 0) ? mesh.uvs[ti] : Maths::UV(0, 0);
                tri[k].normal = model.TransformDirection(vertex_normals[vi]);
            }

            clip_vertex out[6];
            int n_tris = 0;
            clip_triangle_near(tri[0], tri[1], tri[2], out, &n_tris);

            for (int i = 0; i < n_tris; i++) {
                zb.project_and_draw(&out[i * 3], tex, light, half_vec, zb, rtz, fb);
            }
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
                "Chapter 08 — Backface culling  |  culled %d of %d  |  %.1f FPS  (%.2f ms)",
               culled, mesh.faceCount(), fps, ms);
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