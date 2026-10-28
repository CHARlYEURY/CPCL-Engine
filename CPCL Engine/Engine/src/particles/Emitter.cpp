#include "particles/Emitter.h"

float Emitter::rnd()
{
    _rng ^= _rng << 13; _rng ^= _rng >> 17; _rng ^= _rng << 5;   // xorshift32
    return (_rng & 0xFFFFFF) / 16777216.0f;
}

void Emitter::emit(std::vector<Particle>& pool, size_t capacity, float dt)
{
    _accum += rate * dt;
    while (_accum >= 1.0f) {
        _accum -= 1.0f;
        if (pool.size() >= capacity) continue;   // pool plein : on ne spawn plus

        Maths::Vec3_SSE dir = Maths::Vec3_SSE(range(-spread, spread), 1.0f,
            range(-spread, spread)).Normalize();
        Particle p;
        p.pos = position;
        p.vel = dir * range(speedMin, speedMax);
        p.age = 0.0f;
        p.lifetime = range(lifeMin, lifeMax);
        p.size = range(sizeMin, sizeMax);
        p.colorStart = colorStart;
        p.colorEnd = colorEnd;
        pool.push_back(p);
    }
}