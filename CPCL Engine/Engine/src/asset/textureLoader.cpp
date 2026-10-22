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

uint32_t texture_sample_bilinear(const Texture& tex, float u, float v)
{
    u = u - floorf(u);
    v = v - floorf(v);

    float fx = u * tex.width - 1;
    float fy = (1 - v) * tex.height - 1;

    int   x0 = (int)floorf(fx);
    int   y0 = (int)floorf(fy);
    int   x1 = x0 + 1;
    int   y1 = y0 + 1;
    float tx = fx - (float)x0; 
    float ty = fy - (float)y0;

    if (x0 < 0) x0 = 0; if (x0 >= tex.width)  x0 = tex.width - 1;
    if (y0 < 0) y0 = 0; if (y0 >= tex.height) y0 = tex.height - 1;
    if (x1 < 0) x1 = 0; if (x1 >= tex.width)  x1 = tex.width - 1;
    if (y1 < 0) y1 = 0; if (y1 >= tex.height) y1 = tex.height - 1;

    uint32_t c00 = tex.pixels[y0 * tex.width + x0];
    uint32_t c10 = tex.pixels[y0 * tex.width + x1];
    uint32_t c01 = tex.pixels[y1 * tex.width + x0];
    uint32_t c11 = tex.pixels[y1 * tex.width + x1];

    float w00 = (1.0f - tx) * (1.0f - ty);
    float w10 = tx * (1.0f - ty);
    float w01 = (1.0f - tx) * ty;
    float w11 = tx * ty;


    float r = w00 * ((c00 >> 16) & 0xFF) + w10 * ((c10 >> 16) & 0xFF)
        + w01 * ((c01 >> 16) & 0xFF) + w11 * ((c11 >> 16) & 0xFF);
    float g = w00 * ((c00 >> 8) & 0xFF) + w10 * ((c10 >> 8) & 0xFF)
        + w01 * ((c01 >> 8) & 0xFF) + w11 * ((c11 >> 8) & 0xFF);
    float b = w00 * (c00 & 0xFF) + w10 * (c10 & 0xFF)
        + w01 * (c01 & 0xFF) + w11 * (c11 & 0xFF);

    return ((uint32_t)0xFFu << 24)
        | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
   
}
