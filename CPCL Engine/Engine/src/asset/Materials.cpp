#include "asset/Materials.h"

namespace
{
    // Multiplie deux couleurs ARGB canal par canal (0..255 * 0..255 / 255)
    inline uint32_t Modulate(uint32_t a, uint32_t b)
    {
        auto mul = [&](int shift) -> uint32_t
            {
                uint32_t ca = (a >> shift) & 0xFF;
                uint32_t cb = (b >> shift) & 0xFF;
                return (ca * cb + 127) / 255;
            };

        return (mul(24) << 24) | (mul(16) << 16) | (mul(8) << 8) | mul(0);
    }
}

Materials::Materials(std::string name, std::shared_ptr<Texture> texture, uint32_t diffuse)
    : _name(std::move(name)), _texture(std::move(texture)), _diffuse(diffuse) {
}

bool Materials::isTransparent() const
{
    if (hasTexture() && _texture->isTransparent)
        return true;
    
    return ((_diffuse >> 24) & 0xFF) != 255;
}

uint32_t Materials::sampleBaseColor(float u, float v) const
{
    if (!hasTexture())
        return _diffuse;

    return Modulate(_texture->Sample(u, v), _diffuse);
}