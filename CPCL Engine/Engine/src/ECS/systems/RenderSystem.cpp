// ============================================================
//  RenderSystem.cpp
// ============================================================
#include "ECS/systems/RenderSystem.h"


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
    const Maths::Vec3_SSE& light, const Maths::Vec3_SSE& half, const ParticleSystem& ps, float focal)
{
    r.begin_frame(setRGB(16, 16, 24));
    r.draw_particles(ps, proj * view, focal);     // proj * view : les particules sont deja en coordonnées monde
    reg.each<Transform, MeshRenderer>([&](Entity, Transform& t, MeshRenderer& m) {
        // matrice model : rotation Y combinée a la rotation X (position et scale du Transform pas encore pris en compte)
        auto model = Maths::Matrix4x4_SSE::RotationY(t.rotY) * Maths::Matrix4x4_SSE::RotationX(t.rotX);
        auto mvp = proj * (view * model);
        // drawItem = paquet de references pour le Renderer. les pointeurs du MeshRenderer sont deferencés (*)
        // pour donner des references. model et mvp sont des variables locales de la lambda : c'est
        // pas grave car draw_mesh en garde pas de reference, il copie ce qui lui faut dans les ScreenTri
        // (le materiau est stocké par adresse mais c'est l'adresse du vrai Materials, qui vit plus longtemps)
        drawItem item{ *m.mesh, *m.vertexNormals, *m.materials, model, mvp, light, half };
        r.draw_mesh(item, true);
        });
    r.end_Frame();
}