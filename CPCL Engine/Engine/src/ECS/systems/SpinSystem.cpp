#include "ECS/systems/SpinSystem.h"


void SpinSystem(Registry& reg, float dt) {
    reg.each<Transform, Spin>([&](Entity, Transform& t, Spin& s) {
        t.rotX += s.speedX * dt;
        t.rotY += s.speedY * dt;
        });
}