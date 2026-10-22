#pragma once

#include <cstdint>
#include <vector>

struct Texture
{
    std::vector<uint32_t> pixels;
    int width = 0;
    int height = 0;

    uint32_t Sample(float u, float v) const;
};

bool texture_load(const char* path, Texture& out);
uint32_t texture_sample_bilinear(const Texture& tex, float u, float v);
