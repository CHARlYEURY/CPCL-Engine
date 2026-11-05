#include "ECS/systems/RenderSystem.h"


void RenderSystem(Registry& reg, Renderer& r,
    const Maths::Matrix4x4_SSE& view, const Maths::Matrix4x4_SSE& proj,
    const Maths::Vec3_SSE& light, const Maths::Vec3_SSE& half, const ParticleSystem& ps, float focal)
{
    r.begin_frame(setRGB(16, 16, 24));
	r.draw_particles(ps, proj * view, focal);
    reg.each<Transform, MeshRenderer>([&](Entity, Transform& t, MeshRenderer& m) {
        auto model = Maths::Matrix4x4_SSE::RotationY(t.rotY) * Maths::Matrix4x4_SSE::RotationX(t.rotX);
        auto mvp = proj * (view * model);
        drawItem item{ *m.mesh, *m.vertexNormals, *m.materials, model, mvp, light, half }; // prevu pour draw_scene
        r.draw_mesh(item, true);
        });
    r.end_Frame();
}