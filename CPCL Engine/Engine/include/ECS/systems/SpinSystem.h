// ============================================================
//  SpinSystem.h  (système)
// ------------------------------------------------------------
// 
//  fait tourner toutes les entités qui ont un Transform ET un Spin.
//
//  appelé dans main a chaque frame, avant RenderSystem : SpinSystem(reg, dt)
// ============================================================
#pragma once
#include "ECS/Registry.h"
#include "ECS/components/Transform.h"
#include "ECS/components/Spin.h"

// reg = le registry qui contient les entités, dt = temps depuis la derniere frame en secondes
void SpinSystem(Registry& reg, float dt);