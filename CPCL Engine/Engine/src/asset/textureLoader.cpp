#include "asset/textureLoader.h"

#include <cstdio>
#include <cmath>
#include <cstdint>
#include <vector>

// stb_image est une librairie "un seul fichier". Le #define STB_IMAGE_IMPLEMENTATION
// dit "mets le code de la librairie dans CE .cpp" (à faire dans UN SEUL .cpp du projet).
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG   // Les autres #define retirent ce dont on n'a pas besoin : on ne garde que PNG et JPEG
#define STBI_ONLY_JPEG
// compilation plus rapide et exécutable plus petit.

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244 4456 4457 4459 4505 4267)
#endif

#include "stb_image.h"

#ifdef _MSC_VER
#pragma warning(pop)  // veski quelques warnings de Visual Studio de stb_image
#endif

//  Chargement et lecture des textures.
//  Une Texture = un aray de pixels + ratio W/H.
//
//  Contenu :
//    - WrapIndex            : outil pour faire "boucler" un indice
//    - Texture::GetPixel    : lit un pixel (x, y)
//    - Texture::Sample      : lit la couleur à (u, v), plus proche voisin
//    - texture_load         : charge un fichier image avec stb_image
//    - Texture::SampleBilinear : lit à (u, v) avec lissage entre 4 pixels

namespace
{
    inline int WrapIndex(int i, int size) // WrapIndex : fait "boucler" un indice de pixel dans[0, size[
    {
        i %= size;                        //  (le % du C++ peut donner un résultat négatif, d'où la correction)
        return (i < 0) ? i + size : i;    //  Exemples avec size = 10 :  12 -> 2   ;   -1 -> 9
    }
}

// GetPixel : couleur du pixel(x, y) de la texture
uint32_t Texture::GetPixel(int x, int y) const
{
    if (pixels.empty())
        return 0;

    if (x < 0) x = 0;
    if (x >= width) x = width - 1;  //  Si (x, y) sort de l'image, on le ramène au bord (clamp) pour ne
    if (y < 0) y = 0;               //  jamais lire en dehors du tableau.
    if (y >= height) y = height - 1;

    return pixels[static_cast<size_t>(y) * width + x]; //  Le tableau est 1D : indice = ligne * largeur + colonne.
}

//  Sample : couleur à(u, v), avec méthode "plus proche voisin"
//  On prend simplement le pixel le plus proche : rapide, mais pixelisé de près. 
//  Materials::sampleBaseColor. prevu de remplacer par SampleBilinear plus tard.
uint32_t Texture::Sample(float u, float v) const
{
    if (pixels.empty())
        return 0;

    u = u - std::floor(u);  // 1) floor() garde la partie décimale
    v = v - std::floor(v);

    int x = static_cast<int>(u * static_cast<float>(width));            //    2) u * width donne la colonne du pixel
    int y = static_cast<int>((1.0f - v) * static_cast<float>(height));  //    3) v est INVERSÉ : dans un .obj v=0 est en BAS de l'image

    return GetPixel(x, y);
}

//  texture_load : charge une image dans "out"
bool texture_load(const FilePath& path, Texture& out)
{
    int w, h, channels;

    uint8_t* data = stbi_load(path.getString().c_str(), &w, &h, &channels, 4);   //  stbi_load lit le fichier. On demande 4 canaux (RGBA)

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

    for (size_t i = 0; i < out.pixels.size(); i++)  //  on convertit chaque pixel en un uint32_t
    {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = data[i * 4 + 3];

        // Detection de transparence faite pendant la conversion (un seul passage)
        if (a != 255)
            out.isTransparent = true;

        // on assemble en 0xAARRGGBB
        out.pixels[i] =
            (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(r) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(b);
    }

    stbi_image_free(data);  // on libère la mémoire de stb (stbi_image_free)

    return true;
}

bool texture_load(const char* path, Texture& out)   // seconde version, pour compatibilité avec ancienne.
{
    return texture_load(FilePath(path), out);
}


//  SampleBilinear : couleur à (u, v) avec lissage "bilinéaire"
//  Au lieu de prendre UN pixel (Sample), on mélange les 4 pixels autour -> rendu plus doux de près.
//
//        c00 ---- c10        tx, ty = position du point entre les pixels (0 à 1)
//         |   .     |        poids : c00 -> (1-tx)(1-ty)   c10 -> tx(1-ty)
//        c01 ---- c11                c01 -> (1-tx)ty       c11 -> tx*t
//  (Pas utilisée dans le rendu pour l'instant : le matériau appelle Sample.)
uint32_t Texture::SampleBilinear(float u, float v) const
{
    if (pixels.empty())
        return 0;

    u = u - std::floor(u);
    v = v - std::floor(v);

    // Meme convention que Sample (v inverse), mais decalage de 0.5 pixel
    // (centre des pixels)
    float fx = u * static_cast<float>(width) - 0.5f;
    float fy = (1.0f - v) * static_cast<float>(height) - 0.5f;

    int x0 = static_cast<int>(std::floor(fx)); // x0, y0 = pixel en haut à gauche
    int y0 = static_cast<int>(std::floor(fy));
    float tx = fx - static_cast<float>(x0);
    float ty = fy - static_cast<float>(y0);

    // Wrap (repeat) : pas de couture au bord de la texture
    int x1 = WrapIndex(x0 + 1, width);  // x1, y1 = ses voisins
    int y1 = WrapIndex(y0 + 1, height);
    x0 = WrapIndex(x0, width);
    y0 = WrapIndex(y0, height);


    //  on lit les 4 couleurs
    uint32_t c00 = pixels[static_cast<size_t>(y0) * width + x0];
    uint32_t c10 = pixels[static_cast<size_t>(y0) * width + x1];
    uint32_t c01 = pixels[static_cast<size_t>(y1) * width + x0];
    uint32_t c11 = pixels[static_cast<size_t>(y1) * width + x1];

    // on calcule leurs 4 poids (la somme fait 1)
    float w00 = (1.0f - tx) * (1.0f - ty);
    float w10 = tx * (1.0f - ty);
    float w01 = (1.0f - tx) * ty;
    float w11 = tx * ty;

    // lambda : mélange UN canal ("shift" = où il est dans le uint32)
    auto channel = [&](int shift) -> uint32_t
        {
            float c = w00 * static_cast<float>((c00 >> shift) & 0xFF)
                + w10 * static_cast<float>((c10 >> shift) & 0xFF)
                + w01 * static_cast<float>((c01 >> shift) & 0xFF)
                + w11 * static_cast<float>((c11 >> shift) & 0xFF);
            return static_cast<uint32_t>(c + 0.5f); // on mélange canal par canal (A, R, G, B), "+ 0.5" pour arrondir
        };

    return (channel(24) << 24) | (channel(16) << 16) | (channel(8) << 8) | channel(0);
}

// raccourci de SampleBilinear
uint32_t texture_sample_bilinear(const Texture& tex, float u, float v)
{
    return tex.SampleBilinear(u, v);
}