#pragma once

#include <cstdint>
#include <vector>

#include "core/FilePath.h"

struct Texture
{
    std::vector<uint32_t> pixels;
    int width = 0;
    int height = 0;

    FilePath filename;
    bool isTransparent = false;

    [[nodiscard]] bool empty() const { return pixels.empty(); }

    /// Pixel brut (x, y), coordonnees clampees dans la texture.
    [[nodiscard]] uint32_t GetPixel(int x, int y) const;

    /// Echantillonnage plus proche voisin, UV en repeat.
    [[nodiscard]] uint32_t Sample(float u, float v) const;

    /// Echantillonnage bilineaire, UV en repeat.
    [[nodiscard]] uint32_t SampleBilinear(float u, float v) const;
};

bool texture_load(const FilePath& path, Texture& out);
bool texture_load(const char* path, Texture& out);
uint32_t texture_sample_bilinear(const Texture& tex, float u, float v);
