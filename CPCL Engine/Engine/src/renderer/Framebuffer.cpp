// ============================================================
//  Framebuffer.cpp
// ------------------------------------------------------------
//  implementation de la classe FrameBuffer (voir Framebuffer.h).
//  rien de compliqué ici : c'est un tableau de pixels avec quelques fonctions
//  pour le remplir, ecrire un pixel et l'exporter.
// ============================================================
#include "renderer/Framebuffer.h"
#include <algorithm>               


#include <fstream>                

// ------------------------------------------------------------
//  constructeur : memorise la taille et alloue width*height pixels a 0
//  size_t(width) * height : on convertit en size_t AVANT de multiplier
//  pour eviter un depassement d'int sur les grosses images
// ------------------------------------------------------------
FrameBuffer::FrameBuffer(int width, int height) : size_width(width), size_height(height), pixels(size_t(width)* height, 0)
{

}


// ------------------------------------------------------------
//  Clear : met la meme couleur partout
//  appelé au debut de chaque frame par Renderer::begin_frame, sinon on verrait
//  l'image de la frame d'avant derriere les nouveaux objets
// ------------------------------------------------------------
void FrameBuffer::Clear(uint32_t color)
{
    std::fill(pixels.begin(), pixels.end(), color);
}

// ------------------------------------------------------------
//  SetPixel : ecrit un pixel avec verification des bords
//  si (x,y) est en dehors de l'image on sort tout de suite, ca evite d'ecrire
//  en dehors du tableau. conversion de (x,y) en index 1d : y * largeur + x
//  (utilisé par draw_line, le rasterizer de triangles passe lui par operator[] directement)
// ------------------------------------------------------------
void FrameBuffer::SetPixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= size_width || y < 0 || y >= size_height)
        return;
    pixels[size_t(y) * size_width + x] = color;
}

// petits getters pour que les autres classes connaissent la taille (zbuffer, rasterizer, Window...)
int FrameBuffer::getWidth() const
{
    return size_width;
}

int FrameBuffer::getHeight() const
{
    return size_height;
}

// acces au tableau brut, la version const est pour les objets const (ex : Window::Present prend un const FrameBuffer&)
const uint32_t* FrameBuffer::Data() const
{
    return pixels.data();
}

uint32_t* FrameBuffer::Data()
{
    return pixels.data();
}

// ------------------------------------------------------------
//  SavePPM : exporte l'image dans un fichier .ppm (P6 = version binaire)
//  format : une entete texte "P6 largeur hauteur 255" puis les pixels en RGB, 3 octets chacun
//  l'alpha est perdu, le ppm n'en a pas
//  renvoie false si le fichier n'a pas pu etre ouvert
// ------------------------------------------------------------
bool FrameBuffer::SavePPM(const std::string& path) const
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;

    file << "P6\n" << size_width << " " << size_height << "\n255\n";
    for (uint32_t p : pixels)
    {
        // on extrait les canaux de 0xAARRGGBB : decalage puis masque 0xFF pour garder 8 bits
        const char rgb[3] = {
            char((p >> 16) & 0xFF),     // rouge
            char((p >> 8) & 0xFF),      // vert
            char(p & 0xFF)              // bleu
        };
        file.write(rgb, 3);
    }
    return true;
}