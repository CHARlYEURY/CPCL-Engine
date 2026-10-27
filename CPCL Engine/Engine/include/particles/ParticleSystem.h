#pragma once
#include "particles/Emitter.h"

class ParticleSystem
{
private:
    size_t _capacity;

public:
    explicit ParticleSystem(size_t capacity = 10000) : _capacity(capacity)
    {
        particles.reserve(capacity);           // aucune allocation ensuite
    }

    Emitter emitter;
    Maths::Vec3_SSE gravity = Maths::Vec3_SSE(0, -3.0f, 0);
    std::vector<Particle> particles;

    void update(float dt);


};