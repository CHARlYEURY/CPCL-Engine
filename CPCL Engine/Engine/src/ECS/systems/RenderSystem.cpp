// ============================================================
//  RenderSystem.cpp
// ============================================================
#include "ECS/systems/RenderSystem.h"
#include <deque>
#include <vector>

// ------------------------------------------------------------
//  RenderSystem : dessine toute la scene pour une frame
//  ordre des etapes :
//    1) begin_frame : on efface l'image (fond bleu nuit foncé) et le zbuffer
//    2) draw_particles : les particules sont dessinées directement dans le framebuffer
//    3) pour chaque entité Transform + MeshRenderer :
//         - on construit la matrice model a partir de la rotation du Transform
//         - on calcule la mvp = proj * view * model (local -> ecran en une seule matrice)
//         - on met tout dans un drawItem et on l'envoie a draw_mesh
//    4) end_Frame : les triangles stockés sont enfin rasterisés (multi-thread)
// ------------------------------------------------------------
void RenderSystem(Registry& reg, Renderer& r,
    const Maths::Matrix4x4_SSE& view, const Maths::Matrix4x4_SSE& proj,
    const Maths::Vec3_SSE& light, const Maths::Vec3_SSE& half, const ParticleSystem& ps, float focal, const Maths::Vec3_SSE& camera_pos)
{

    struct Matrixs { Maths::Matrix4x4_SSE model, mvp; };
    std::deque<Matrixs> matrixs;
    std::vector<drawItem> scene;

    r.begin_frame(setRGB(16, 16, 24));
    
    reg.each<Transform, MeshRenderer>([&](Entity, Transform& t, MeshRenderer& m) {
        // matrice model : rotation Y combinée a la rotation X (position et scale du Transform pas encore pris en compte)
        auto model =  Maths::Matrix4x4_SSE::Translation(t.position) 
                    * Maths::Matrix4x4_SSE::RotationY(t.rotY)
                    * Maths::Matrix4x4_SSE::RotationX(t.rotX);
		matrixs.push_back({ model, proj * view * model });  // on stocke les matrices pour la lambda suivante (on peut pas capturer par reference car elles sont locales a la lambda)
        const Matrixs& mm = matrixs.back();
		scene.push_back({ *m.mesh, *m.vertexNormals, *m.materials, mm.model, mm.mvp, light, half });
        });
        
    r.draw_scene(scene, camera_pos);
    r.end_Frame();
    r.draw_particles(ps, proj* view, focal);     // proj * view : les particules sont deja en coordonnées monde
}