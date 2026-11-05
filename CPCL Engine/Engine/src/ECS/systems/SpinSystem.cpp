// ============================================================
//  SpinSystem.cpp
// ============================================================
#include "ECS/systems/SpinSystem.h"


// ------------------------------------------------------------
//  SpinSystem : fait avancer la rotation de chaque entité qui a Transform + Spin
//  each<Transform, Spin> appelle la lambda pour chaque entité qui possede les deux,
//  avec une reference directe sur ses composants (on les modifie sur place).
//  rotation += vitesse * dt : on multiplie par dt pour que la vitesse soit en
//  radians PAR SECONDE, donc independante du nombre de fps
//  [&] = la lambda capture dt par reference, le parametre Entity n'est pas utilisé (pas de nom)
// ------------------------------------------------------------
void SpinSystem(Registry& reg, float dt) {
    reg.each<Transform, Spin>([&](Entity, Transform& t, Spin& s) {
        t.rotX += s.speedX * dt;
        t.rotY += s.speedY * dt;
        });
}