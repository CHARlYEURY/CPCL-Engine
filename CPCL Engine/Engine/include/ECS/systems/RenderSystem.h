#pragma once 
#include "ECS/Registry.h"
#include "ECS/components/Transform.h"
#include "ECS/components/MeshRenderer.h"
#include "renderer/Renderer.h"
#include "Matrix4x4_SSE.h"
#include "Vec3_SSE.h"


void RenderSystem(Registry& reg, Renderer& r,
    const Maths::Matrix4x4_SSE& view, const Maths::Matrix4x4_SSE& proj,
    const Maths::Vec3_SSE& light, const Maths::Vec3_SSE& half, const ParticleSystem& ps, float focal);