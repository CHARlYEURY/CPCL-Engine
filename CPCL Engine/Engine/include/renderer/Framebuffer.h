// ============================================================
//  Framebuffer.h
// ------------------------------------------------------------
//  l'IMAGE finale un grand tableau de pixels (un uint32_t par pixel)
//  dans lequel le rasterizer ecrit les couleurs. a la fin de la frame on l'envoie
//  a la fenetre (Window::Present) qui l'affiche a l'ecran.
//
//  format d'un pixel : 0xAARRGGBB  (alpha, rouge, vert, bleu, 8 bits chacun)
//  c'est le meme format que SDL_PIXELFORMAT_ARGB8888 utilisé dans Window.cpp,
//  donc on peut copier le tableau tel quel dans la texture SDL sans conversion.
//
//  possedé par le Renderer (membre _fb), le Rasterizer et draw_particles
//  ecrivent dedans, et main le recupere via Renderer::framebuffer().
// ============================================================
#pragma once
#include <cstdint>
#include <vector>
#include <string>

// ------------------------------------------------------------
//  setRGB : assemble 4 canaux (0..255) en un seul uint32_t 0xAARRGGBB
//  constexpr = peut etre calculée a la compilation, alpha a 255 (opaque) par defaut
//  decalages : a -> bits 24-31, r -> 16-23, g -> 8-15, b -> 0-7
//  utilisée un peu partout (clear color, blend_colors, shade...)
// ------------------------------------------------------------

constexpr uint32_t setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
	return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

class FrameBuffer
{
private:

	int size_width, size_height;           // dimensions de l'image en pixels
	std::vector <uint32_t> pixels;         // tous les pixels a la suite, ligne par ligne (index = y * largeur + x)
public:

	// cree une image width x height, tous les pixels sont a 0 au depart
	FrameBuffer(int width, int height);

	// Accès en écriture
	// permet d'ecrire fb[idx] = couleur, idx est deja l'index 1d (y * largeur + x)
	// c'est ce qu'utilise le rasterizer dans sa boucle (pas de verification des bords ici, pour aller vite)
	uint32_t& operator[](size_t idx) { return pixels[idx]; }
	// Accès en lecture (objets const)
	const uint32_t& operator[](size_t idx) const { return pixels[idx]; }


	// remplit toute l'image avec une couleur (appelé par Renderer::begin_frame)
	void Clear(uint32_t color);
	// ecrit UN pixel a la position (x, y), avec verification des bords : hors ecran = on fait rien
	// utilisé par draw_line, la version "sure" de operator[]
	void SetPixel(int x, int y, uint32_t color);

	int getWidth() const;
	int getHeight() const;
	// pointeur brut vers le tableau de pixels, utilisé par Window::Present (SDL_UpdateTexture)
	// version const (lecture seule) et version normale (ecriture), par exemple pour text_blit
	const uint32_t* Data() const;
	uint32_t* Data();

	// Export pour vérifier le rendu sans fenêtre
	// sauvegarde l'image en .ppm (format tout simple), renvoie false si le fichier peut pas etre ouvert
	bool SavePPM(const std::string& path) const;
};