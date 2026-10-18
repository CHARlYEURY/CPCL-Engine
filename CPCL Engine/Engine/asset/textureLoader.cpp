#include "textureLoader.h"

#include <cstdio>
#include <cmath>
#include <cstdint>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244 4456 4457 4459 4505 4267)
#endif

#include "stb_image.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

uint32_t Texture::Sample(float u, float v) const
{
    u = u - std::floor(u);
    v = v - std::floor(v);

    int x = static_cast<int>(u * static_cast<float>(width));
    int y = static_cast<int>((1.0f - v) * static_cast<float>(height));

    if (x < 0) x = 0;
    if (x >= width) x = width - 1;

    if (y < 0) y = 0;
    if (y >= height) y = height - 1;

    return pixels[static_cast<size_t>(y) * width + x];
}

bool texture_load(const char* path, Texture& out)
{
    int w, h, channels;

    uint8_t* data = stbi_load(path, &w, &h, &channels, 4);

    if (!data)
    {
        std::fprintf(
            stderr,
            "texture_load: failed on '%s': %s\n",
            path,
            stbi_failure_reason()
        );

        return false;
    }

    out.width = w;
    out.height = h;
    out.pixels.resize(static_cast<size_t>(w) * h);

    for (size_t i = 0; i < out.pixels.size(); i++)
    {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];

        out.pixels[i] =
            (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }

    stbi_image_free(data);

    return true;
}