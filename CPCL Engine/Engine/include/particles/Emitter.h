#pragma once
#include <vector>
#include <cstdint>
#include "Vec3_SSE.h"

struct Particle {
    Maths::Vec3_SSE pos, vel;
    float age, lifetime, size;
    uint32_t colorStart, colorEnd;
};

class Emitter
{
public:
    Maths::Vec3_SSE position = Maths::Vec3_SSE(0, 0, 0);
    float rate = 300.0f;                       // particules / seconde
    float speedMin = 2.0f, speedMax = 3.5f;
    float lifeMin = 1.0f, lifeMax = 2.0f;
    float sizeMin = 0.03f, sizeMax = 0.08f;    // taille en unités monde
    float spread = 0.4f;                       // 0 = jet droit, 1 = très ouvert
    uint32_t colorStart = 0xFFFFC040;          // 0xAARRGGBB
    uint32_t colorEnd = 0xFFFF2000;

    void emit(std::vector<Particle>& pool, size_t capacity, float dt);

private:
    float _accum = 0.0f;
    uint32_t _rng = 12345u;
    float rnd();                               // [0,1[
    float range(float a, float b) { return a + (b - a) * rnd(); }
};