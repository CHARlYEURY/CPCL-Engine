#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "asset/textureLoader.h"


class Materials final
{
private:
    std::string _name;                        ///< Nom (utile plus tard pour le loader de mesh : usemtl)
    std::shared_ptr<Texture> _texture;        ///< Texture optionnelle (nullptr = couleur unie)
    uint32_t _diffuse = 0xFFFFFFFF;           ///< Couleur de base (diffuse, utilisee par Lambert)

public:
    explicit Materials(std::string name = "",
        std::shared_ptr<Texture> texture = nullptr,
        uint32_t diffuse = 0xFFFFFFFF);

    [[nodiscard]] const std::string& name() const { return _name; }
    [[nodiscard]] uint32_t diffuse() const { return _diffuse; }
    [[nodiscard]] std::shared_ptr<Texture> texture() const { return _texture; }
    [[nodiscard]] bool hasTexture() const { return _texture != nullptr && !_texture->empty(); }

    /// true si la texture a de la transparence, ou si la couleur diffuse a alpha != 255.
    [[nodiscard]] bool isTransparent() const;
    
    void setDiffuse(uint32_t color) { _diffuse = color; }
    void setTexture(std::shared_ptr<Texture> texture) { _texture = std::move(texture); }

    /// Couleur de base a l'UV donne : texture (si presente) multipliee par la couleur diffuse.
    [[nodiscard]] uint32_t sampleBaseColor(float u, float v) const;
};