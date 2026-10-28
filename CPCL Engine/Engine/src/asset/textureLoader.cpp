#include "asset/textureLoader.h"

#include <cstdio>
#include <cmath>
#include <cstdint>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244 4456 4457 4459 4505 4267)
#endif

#include "stb_image.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace
{
    // Wrap "repeat" d'un indice de pixel
    inline int WrapIndex(int i, int size)
    {
        i %= size;
        return (i < 0) ? i + size : i;
    }
}

uint32_t Texture::GetPixel(int x, int y) const
{
    if (pixels.empty())
        return 0;

    if (x < 0) x = 0;
    if (x >= width) x = width - 1;
    if (y < 0) y = 0;
    if (y >= height) y = height - 1;

    return pixels[static_cast<size_t>(y) * width + x];
}


uint32_t Texture::Sample(float u, float v) const
{
    if (pixels.empty())
        return 0;

    u = u - std::floor(u);
    v = v - std::floor(v);

    int x = static_cast<int>(u * static_cast<float>(width));
    int y = static_cast<int>((1.0f - v) * static_cast<float>(height));

    return GetPixel(x, y);
}

bool texture_load(const FilePath& path, Texture& out)
{
    int w, h, channels;

    uint8_t* data = stbi_load(path.getString().c_str(), &w, &h, &channels, 4);

    if (!data)
    {
        std::fprintf(
            stderr,
            "texture_load: failed on '%s': %s\n",
            path.getString().c_str(),
            stbi_failure_reason()
        );

        return false;
    }

    out.width = w;
    out.height = h;
    out.filename = path;
    out.isTransparent = false;
    out.pixels.resize(static_cast<size_t>(w) * h);

    for (size_t i = 0; i < out.pixels.size(); i++)
    {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];

        // Detection de transparence faite pendant la conversion (un seul passage)
        if (a != 255)
            out.isTransparent = true;

        out.pixels[i] =
            (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }

    stbi_image_free(data);

    return true;
}

bool texture_load(const char* path, Texture& out)
{
    return texture_load(FilePath(path), out);
}

uint32_t Texture::SampleBilinear(float u, float v) const
{
    if (pixels.empty())
        return 0;

    u = u - std::floor(u);
    v = v - std::floor(v);

    // Meme convention que Sample (v inverse), mais decalage de 0.5 pixel
    // (centre des pixels) au lieu de 1 comme dans l'ancienne version.
    float fx = u * static_cast<float>(width) - 0.5f;
    float fy = (1.0f - v) * static_cast<float>(height) - 0.5f;

    int x0 = static_cast<int>(std::floor(fx));
    int y0 = static_cast<int>(std::floor(fy));
    float tx = fx - static_cast<float>(x0);
    float ty = fy - static_cast<float>(y0);

    // Wrap (repeat) : pas de couture au bord de la texture
    int x1 = WrapIndex(x0 + 1, width);
    int y1 = WrapIndex(y0 + 1, height);
    x0 = WrapIndex(x0, width);
    y0 = WrapIndex(y0, height);

    uint32_t c00 = pixels[static_cast<size_t>(y0) * width + x0];
    uint32_t c10 = pixels[static_cast<size_t>(y0) * width + x1];
    uint32_t c01 = pixels[static_cast<size_t>(y1) * width + x0];
    uint32_t c11 = pixels[static_cast<size_t>(y1) * width + x1];

    float w00 = (1.0f - tx) * (1.0f - ty);
    float w10 = tx * (1.0f - ty);
    float w01 = (1.0f - tx) * ty;
    float w11 = tx * ty;

    auto channel = [&](int shift) -> uint32_t
        {
            float c = w00 * static_cast<float>((c00 >> shift) & 0xFF)
                + w10 * static_cast<float>((c10 >> shift) & 0xFF)
                + w01 * static_cast<float>((c01 >> shift) & 0xFF)
                + w11 * static_cast<float>((c11 >> shift) & 0xFF);
            return static_cast<uint32_t>(c + 0.5f);
        };

    return (channel(24) << 24) | (channel(16) << 16) | (channel(8) << 8) | channel(0);
}


uint32_t texture_sample_bilinear(const Texture& tex, float u, float v)
{
    return tex.SampleBilinear(u, v);
}