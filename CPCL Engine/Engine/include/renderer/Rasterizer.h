// ============================================================
//  Rasterizer.h
// ------------------------------------------------------------
//  c'est ici qu'on transforme un triangle (3 points en pixels) en vrais pixels
//  colorés dans le framebuffer. le rasterizer fait :
//     - le test "est ce que ce pixel est dans le triangle" (edge functions)
//     - l'interpolation de la profondeur, des uv et des normales (coordonnées barycentriques)
//     - le z-test avec le zbuffer
//     - l'éclairage phong (diffus + speculaire) et la texture
//     - le blending pour les materiaux transparents
//
//  il ne decide PAS quels triangles dessiner, ca c'est le boulot du Renderer
//  (clipping, projection, culling, binning). le Renderer lui envoie des ScreenTri
//  deja prets via draw_triangle_phong, tuile par tuile, depuis plusieurs threads.
// ============================================================
#pragma once
#include "renderer/Framebuffer.h"      
#include "renderer/Z_buffer.h"        
#include "asset/textureLoader.h"
#include "Vec3_SSE.h"
#include "asset/Materials.h"

// rectangle d'une tuile de l'ecran, en pixels
// (le Renderer decoupe l'ecran en tuiles de 64x64 pour les repartir entre les threads)
struct TileRect { int x0, y0, x1, y1; };   // x1,y1 exclus

// ------------------------------------------------------------
//  ScreenTri : un triangle DEJA projeté a l'ecran, pret a etre rasterisé.
//  construit dans Renderer::project_and_draw, consommé dans draw_triangle_phong.
//  chaque tableau a 3 cases = une par sommet.
// ------------------------------------------------------------
struct ScreenTri {
	int x[3], y[3];               // position du sommet en pixels (entiers)
	float z[3], iw[3];            // z = profondeur ndc (pour le zbuffer), iw = 1/w du clip space (pour la perspective correcte)
	Maths::UV uv[3];              // coordonnées de texture de chaque sommet
	Maths::Vec3_SSE n[3];         // normale de chaque sommet, deja en espace monde
	const Materials* mat;         // materiau du triangle (pointeur : il appartient pas au ScreenTri)
	Maths::Vec3_SSE L, H;         // L = direction de la lumiere, H = half vector (identiques pour les 3 sommets)
};

class Rasterizer
{
private:
	// references vers le framebuffer et le zbuffer du Renderer (il les possede, nous on fait que les utiliser)
	FrameBuffer& _mFramebuffer;
	ZBuffer& _mZBuffer;
public:

	Rasterizer(FrameBuffer& framebuffer, ZBuffer& zbuffer) : _mFramebuffer(framebuffer), _mZBuffer(zbuffer) {};

	// trace une ligne de 1 pixel de large entre deux points (bresenham)
	// pas utilisée dans le rendu des triangles, c'est un outil de debug / fil de fer
	void draw_line(int x0, int y0, int x1, int y1, uint32_t color);

	// edge function : 2x l'aire signée du triangle (a, b, p), le signe dit de quel coté de l'arete ab est p
	// utilisée par draw_triangle_phong (pixel dedans ? + poids barycentriques)
	// et par Renderer::project_and_draw (back-face culling)
	int edge(int ax, int ay, int bx, int by, int px, int py);

	// ancienne interpolation de couleur par sommet (gouraud), remplacée par le phong par pixel
	// n'est plus appelée par draw_triangle_phong mais je la garde pour l'instant
	uint32_t shade(int e0, int e1, int e2, int wsum, uint32_t c0, uint32_t c1, uint32_t c2);

	// rasterise un triangle avec Z-test, texture perspective-correcte et Phong
	// (framebuffer et zbuffer sont maintenant des membres : plus besoin de les passer en paramètre)
	// tri = triangle a dessiner, r = tuile courante, writeDepth = true -> on met a jour le zbuffer
	// appelée par Renderer::end_Frame (dans les threads)
	void draw_triangle_phong(const ScreenTri& tri, const TileRect& r, bool writeDepth = true);
};