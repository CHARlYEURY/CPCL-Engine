#include "particles/ParticleSystem.h"

void ParticleSystem::update(float dt)
{
    emitter.emit(particles, _capacity, dt);

    for (size_t i = 0; i < particles.size(); ) {
        Particle& p = particles[i];
        p.age += dt;
        if (p.age >= p.lifetime) {
            p = particles.back();              // swap-remove, on ne ré-incrémente pas i
            particles.pop_back();
            continue;
        }
        p.vel = p.vel + gravity * dt;
        p.pos = p.pos + p.vel * dt;
        ++i;
    }
}