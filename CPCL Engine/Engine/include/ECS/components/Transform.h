// ============================================================
//  Transform.h  (composant)
// ------------------------------------------------------------
//  place un objet dans le monde 3d : position, rotation, taille.
//  c'est juste des données, aucune logique (la logique est dans les systèmes).
//
//  modifié par : SpinSystem (fait tourner rotX / rotY)
//  lu par      : RenderSystem (construit la matrice model a partir de la rotation)
// ============================================================
#pragma once 
#include "Vec3_SSE.h"

struct Transform {
    Maths::Vec3_SSE position{ 0, 0, 0 };     // position dans le monde (pas encore utilisée par RenderSystem)
    float rotX = 0, rotY = 0;                // simple pour commencer
  // rotation en radians autour de X et de Y (pas de Z pour l'instant)
    float scale = 1;               // taille uniforme (pas encore utilisée par RenderSystem non plus)
};