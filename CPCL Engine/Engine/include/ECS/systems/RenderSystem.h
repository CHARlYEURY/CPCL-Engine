#pragma once 
#include "ECS/Registry.h"
#include "ECS/components/Transform.h"
#include "ECS/components/MeshRenderer.h"
#include "renderer/Renderer.h"
#include "Matrix4x4_SSE.h"
#include "Vec3_SSE.h"


void RenderSystem(Registry& reg, Renderer& r,
    const Maths::Matrix4x4_SSE& view, const Maths::Matrix4x4_SSE& proj,
    const Maths::Vec3_SSE& light, const Maths::Vec3_SSE& half)
{
    r.begin_frame(setRGB(16, 16, 24));
    reg.each<Transform, MeshRenderer>([&](Entity, Transform& t, MeshRenderer& m) {
        auto model = Maths::Matrix4x4_SSE::RotationY(t.rotY) * Maths::Matrix4x4_SSE::RotationX(t.rotX);
        auto mvp = proj * (view * model);
        r.draw_mesh(*m.mesh, *m.vertexNormals, *m.materials, model, mvp, light, half);
        });
    r.end_Frame();
}