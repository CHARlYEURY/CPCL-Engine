// ============================================================
//  Z_buffer.h
// ------------------------------------------------------------
//  le zbuffer (ou depth buffer) garde la PROFONDEUR du pixel le plus proche
//  deja dessiné, pour chaque pixel de l'ecran. il a la meme taille que le framebuffer.
//
//  pourquoi : sans lui il faudrait trier tous les triangles du plus loin au plus proche.
//  avec lui on dessine dans n'importe quel ordre : avant d'ecrire un pixel on compare
//  sa profondeur a celle qui est deja stockée, et on garde que le plus proche.
//
//  convention ici : plus la valeur est PETITE, plus c'est proche de la camera.
//  valeur de depart = 1e30 (un "infini") donc le premier triangle gagne toujours.
//
//  possedé par le Renderer (membre _zb), lu / ecrit par le Rasterizer
//  (z-test dans draw_triangle_phong) et lu par draw_particles.
// ============================================================
#pragma once
#include <vector>

// Ne contient plus que le buffer de profondeur.
// Le rasterisation est dans Rasterizer, l'éclairage dans Lighting, les normales dans Mesh.
class ZBuffer
{
private:
	int Widths, Heights;               // dimensions, identiques a celles du framebuffer
	std::vector<float> zbuffer;        // une profondeur (float) par pixel, index = y * largeur + x
public:

	// alloue width*height floats, tous initialisés a 1e30f (= "rien dessiné ici, tres loin")
	ZBuffer(int width, int height)
		: Widths(width), Heights(height), zbuffer(width* height, 1e30f)
	{
	}

	// Même principe que FrameBuffer::operator[]
	// acces direct par index 1d, pas de verification des bords (pour aller vite dans la boucle du rasterizer)
	float& operator[](size_t idx) { return zbuffer[idx]; }
	const float& operator[](size_t idx) const { return zbuffer[idx]; }

	// le rasterizer en a besoin pour calculer l'index et clamper sa bounding box
	int getWidth() const { return Widths; }
	int getHeight() const { return Heights; }

	// remet tout a 1e30f, a appeler au debut de chaque frame (Renderer::begin_frame)
	void clear_Zbuffer();
};