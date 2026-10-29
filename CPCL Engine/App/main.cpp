#include <iostream>
#include <SDL3/SDL.h>
#include "renderer/Renderer.h"
#include "Vec3_SSE.h"
#include "Matrix4x4_SSE.h"
#include <numbers>
#include "asset/AssetLoader.h"
#include "asset/textureLoader.h"
#include "asset/Font.h"
#include "asset/TextRenderer.h"
#include "platform/Window.h"
#include "particles/ParticleSystem.h"
#include "particles/Emitter.h"
#include "camera/camera.h"
#include <memory>


 
 const int WIDTH = 800;
 const int HEIGHT = 600;
 const int SCALE = 1;  
 const float PI = std::numbers::pi;
 
  


int main() {
    
	/////////////////////////////////////////////////////////// pas faire gaffe ////////////////////////////////////////////////////////////
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    // test Font loading
    if (!TTF_Init()) {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << "\n";
    }
    
    auto font = std::make_unique<Font>(FontTag("default"), 
        FilePath("Assets/Fonts", "BILLO___.TTF"));

    // Fenêtre + renderer SDL + texture qui reçoit les pixels du framebuffer (voir platform/Window)
    Window window("CPU Renderer", WIDTH * SCALE, HEIGHT * SCALE, WIDTH, HEIGHT);
    if (!window.IsValid())
    {
        std::cerr << "Creation fenetre/renderer: " << SDL_GetError() << "\n";
        return 1;
    }

    const Uint64 freq = SDL_GetPerformanceFrequency();
    const Uint64 start = SDL_GetPerformanceCounter();
    Uint64 last = start;
    double fps_accum = 0.0;
    int    fps_count = 0;
    char   title[1600];

    /////////////////////////////////////////////////////////// pas faire gaffe ////////////////////////////////////////////////////////////

    // framebuffer + zbuffer + rasterizer, regroupés dans le Renderer
    Renderer renderer(WIDTH, HEIGHT);

    auto texMat = std::make_shared<Texture>();
    if (!texture_load(FilePath("Assets/Materials", "Ground054_1K-JPG_Color.jpg"), *texMat)) {
        return -1;
    }
    Materials mat("mat");
    mat.setDiffuse(setRGB(255, 255, 255, 128));

    //test
    auto spotTex = std::make_shared<Texture>();
    if (!texture_load("Assets/spot_texture.png", *spotTex)) return 1;

    Materials spotMat("spot", spotTex);
    //spotMat.setDiffuse(setRGB(255, 255, 255, 128));

    Mesh mesh;
    if (!mesh_load("Assets/spot.obj", mesh)) return 1;
    printf("loaded spot.obj: %d vertices, %d faces\n",
        mesh.vertCount(), mesh.faceCount());

    Mesh mesh2;
    if (!mesh_load("Assets/spot.obj", mesh2)) return 1;
    printf("loaded spot.obj: %d vertices, %d faces\n",
        mesh2.vertCount(), mesh2.faceCount());;

    std::vector<Maths::Vec3_SSE> face_normals;
    std::vector<Maths::Vec3_SSE> vertex_normals;

    std::vector<Maths::Vec3_SSE> face_normals2;
    std::vector<Maths::Vec3_SSE> vertex_normals2;

    // Normales calculées UNE fois, en espace objet : le mesh est rigide, elles ne
    // changent pas. Seule la rotation du modèle les fera tourner (plus bas)
    compute_face_normals(mesh, face_normals);
    compute_vertex_normals(mesh, face_normals, vertex_normals);

    compute_face_normals(mesh2, face_normals2);
    compute_vertex_normals(mesh2, face_normals2, vertex_normals2);

    // Éclairage en espace MONDE. Ces vecteurs pointent VERS la lumière / la caméra
    const Maths::Vec3_SSE light = Maths::Vec3_SSE(0.4f, 0.8f, 0.45f).Normalize();
    const Maths::Vec3_SSE view_dir(0, 0, 1);
    const Maths::Vec3_SSE half_vec = (light + view_dir).Normalize();


    ////////////////////// CAMERA ////////////////////////////
    const float aspect = (float)WIDTH / (float)HEIGHT;
    const float fovY = 60.0f * PI / 180.0f;
    
    const Maths::Matrix4x4_SSE projection = Maths::Matrix4x4_SSE::Perspective(fovY, aspect,
        0.1f, 100.0f);
   
    const auto eye = Maths::Vec3_SSE(0, 0, 3);
    const auto target = Maths::Vec3_SSE(0, 0, 0);
    const auto  up = Maths::Vec3_SSE(0, 1, 0);
    //const auto view = Maths::Matrix4x4_SSE::LookAt(eye, target, up);
	Cam camera(eye);
    ////////////////////// CAMERA ////////////////////////////


    //Text font
    SDL_Surface* helloText = text_make_surface(*font, "Hello CPU Engine", 32,
        SDL_Color{ 255, 255, 255, 255 });


    ParticleSystem particles(10000);
    particles.emitter.position = Maths::Vec3_SSE(0, -1.0f, 0);
    const float focal = HEIGHT * 0.5f / tanf(fovY * 0.5f);
    Uint64 lastFrame = start;


    auto f = camera.forward();
    printf("forward = %f %f %f\n", f.reg.m128_f32[0], f.reg.m128_f32[1], f.reg.m128_f32[2]);


    /////////////////////// RUN /////////////////////
    bool running = true;
	bool isRightMouseButtonDown = false;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    isRightMouseButtonDown = true;
                    SDL_SetWindowRelativeMouseMode(window.Get(), true);
				}
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    isRightMouseButtonDown = false;
                    SDL_SetWindowRelativeMouseMode(window.Get(), false);
                }
			}
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (isRightMouseButtonDown) {
                    camera.rotate(event.motion.xrel, event.motion.yrel);
                }
            }
        }

        Uint64 nowF = SDL_GetPerformanceCounter();
        float sim_dt = std::min(0.05f, (float)((double)(nowF - lastFrame) / (double)freq));
        lastFrame = nowF;
        particles.update(sim_dt);
		if (isRightMouseButtonDown)
            camera.update(sim_dt);
		const auto view = camera.viewMatrix();
       
        double seconds = (double)(SDL_GetPerformanceCounter() - start) / (double)freq;
        float t = (float)seconds;

        

        // Matrice modèle : se lit de droite à gauche → d'abord RotationX, puis RotationY
        Maths::Matrix4x4_SSE model = Maths::Matrix4x4_SSE::RotationY(t * 0.8f) * Maths::Matrix4x4_SSE::RotationX(t * 0.5f);
        Maths::Matrix4x4_SSE mvp = projection * (view * model);

        Maths::Vec3_SSE pointMesh = Maths::Vec3_SSE(0, 0, 1);
        Maths::Matrix4x4_SSE model2 = Maths::Matrix4x4_SSE::Translation(pointMesh);
        Maths::Matrix4x4_SSE mvp2 = projection * (view * model2);
        

		int culled = 0;

        std::vector<drawItem> scene{
            {mesh, vertex_normals, spotMat, model, mvp, light, half_vec},
            {mesh2, vertex_normals2, mat, model2, mvp2, light, half_vec}
        };

        // clipping near + projection + back-face culling + rasterisation (voir renderer/Renderer)
        renderer.begin_frame(setRGB(16, 16, 24));
        renderer.draw_scene(scene, camera.position());
        renderer.end_Frame();
        renderer.draw_particles(particles, projection * view, focal);


        text_blit(helloText, renderer.framebuffer().Data(), WIDTH, HEIGHT, 10, 10);

        // --- Affichage ---
        window.Present(renderer.framebuffer());

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
                "Chapter 08 — Backface culling  |  culled %d of %d  |  %.1f FPS  (%.2f ms) | %d particles ",
               culled, mesh.faceCount(), fps, ms, particles.particles.size());
            window.SetTitle(title);
            fps_accum = 0.0;
            fps_count = 0;

        }
        
    }

    SDL_DestroySurface(helloText);
    font.reset();      // détruit la Font -> TTF_CloseFont, AVANT TTF_Quit
    TTF_Quit();
    window.Destroy();  // texture + renderer + fenêtre, AVANT SDL_Quit
    SDL_Quit();
    return 0;
}
