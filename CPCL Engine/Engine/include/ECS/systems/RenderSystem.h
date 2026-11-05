// ============================================================
//  RenderSystem.h  (système)
// ------------------------------------------------------------
//  c'est le "pont" entre l'ECS et le Renderer : il parcourt les entités visibles
//  (Transform + MeshRenderer), prépare pour chacune ses matrices et un drawItem,
//  et demande au Renderer de la dessiner.
//
//  appelé dans main a chaque frame, apres SpinSystem et ParticleSystem::update :
//  RenderSystem(reg, renderer, view, proj, light, half, ps, focal)
// ============================================================
#pragma once 
#include "ECS/Registry.h"
#include "ECS/components/Transform.h"
#include "ECS/components/MeshRenderer.h"
#include "renderer/Renderer.h"
#include "Matrix4x4_SSE.h"
#include "Vec3_SSE.h"


// reg   = les entités a dessiner
// r     = le Renderer qui fait le vrai travail
// view  = matrice de la camera (Cam::viewMatrix), proj = matrice de projection perspective
// light = direction de la lumiere, half = half vector pour le speculaire (les deux calculés dans main)
// ps    = le systeme de particules a dessiner
// focal = focale en pixels, sert a la taille des particules
void RenderSystem(Registry& reg, Renderer& r,
    const Maths::Matrix4x4_SSE& view, const Maths::Matrix4x4_SSE& proj,
    const Maths::Vec3_SSE& light, const Maths::Vec3_SSE& half, const ParticleSystem& ps, float focal);