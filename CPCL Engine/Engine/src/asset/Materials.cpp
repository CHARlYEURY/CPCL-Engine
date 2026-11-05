#include "asset/Materials.h"

//  Un matériau décrit "de quoi est fait" le dessus d'un objet :
//    - une couleur de base (_diffuse), au format 0xAARRGGBB
//    - une texture optionnelle (_texture), nullptr = couleur unie
// 
//  Le rasterizer appelle sampleBaseColor(u, v) pour chaque pixel
//  afin de savoir quelle couleur de base utiliser avant l'éclairage.
//
//  Contenu :
//    - Modulate (outil interne) : multiplie deux couleurs
//    - Materials::isTransparent 
//    - Materials::sampleBaseColor : couleur de base à un point (u, v)


namespace // Le namespace anonyme = fonctions visibles uniquement dans CE fichier
{
    //  Modulate : multiplie deux couleurs ARGB canal avec canal.
    //  Exemple : texture rouge (255,0,0) * diffuse gris (128,128,128)
    //            = rouge foncé (128,0,0)
    // 
    // Le "+ 127" sert à ARRONDIR au plus proche au lieu de tronquer vers le bas.
    //  Ça traite 4 canaux (A, R, G, B) : les décalages 24, 16, 8 et 0.
    inline uint32_t Modulate(uint32_t a, uint32_t b)
    {
		// lambda : calcule UN canal ; "shift" dit où il se trouve dans le uint32_t
        auto mul = [&](int shift) -> uint32_t
            {
                uint32_t ca = (a >> shift) & 0xFF;  // canal de la couleur a
                uint32_t cb = (b >> shift) & 0xFF;  // canal de la couleur b
                return (ca * cb + 127) / 255;
            };

        return (mul(24) << 24) | (mul(16) << 16) | (mul(8) << 8) | mul(0);
    }
}


//  std::move évite de copier la string et le shared_ptr inutilement.
Materials::Materials(std::string name, std::shared_ptr<Texture> texture, uint32_t diffuse)
    : _name(std::move(name)), _texture(std::move(texture)), _diffuse(diffuse) {
}


bool Materials::isTransparent() const
{
    if (hasTexture() && _texture->isTransparent)
        return true;
    
    return ((_diffuse >> 24) & 0xFF) != 255;
}

//  sampleBaseColor : couleur de base du matériau au point
//    - sans texture : juste la couleur diffuse
//    - avec texture : la couleur du texel multipliée par la diffuse
//  Sample au lieu de sampleBilinear à changer!!!
uint32_t Materials::sampleBaseColor(float u, float v) const
{
    if (!hasTexture())
        return _diffuse;

    return Modulate(_texture->Sample(u, v), _diffuse);
}