// ============================================================
//  Renderer.h
// ------------------------------------------------------------
//  c'est le "chef d'orchestre" du rendu 3d (rasterisation soft, tout sur cpu).
//  il possede le framebuffer, le zbuffer et le rasterizer, et il enchaine
//  les etapes du pipeline pour chaque frame :
//
//     begin_frame  -> on vide l'ecran et le zbuffer
//     draw_mesh    -> pour chaque objet : transformation, clipping near,
//                     projection ecran, back-face culling, puis on STOCKE
//                     les triangles (on dessine pas encore)
//     end_Frame    -> on range les triangles par tuile (binning) puis on
//                     rasterise toutes les tuiles en multi-thread
//
//  utilise par : RenderSystem (ECS) qui appelle begin_frame / draw_particles /
//  draw_mesh / end_Frame dans cet ordre, et par main pour recuperer le
//  framebuffer et l'envoyer a la fenetre (Window::Present).
// ============================================================

#pragma once
#include <vector>
#include <cstdint>
#include "renderer/Framebuffer.h"      
#include "renderer/Z_buffer.h"         
#include "renderer/Rasterizer.h"       
#include "renderer/clip.h"             
#include "asset/Mesh.h"
#include "asset/textureLoader.h"
#include "Vec3_SSE.h"
#include "Matrix4x4_SSE.h"
#include "particles/ParticleSystem.h"
#include <atomic>                    
#include <thread>
#include <algorithm>


// ------------------------------------------------------------
//  drawItem : tout ce qu'il faut pour dessiner UN objet pendant UNE frame.
//  ce sont que des references, rien n'est copié, donc les objets pointés
//  doivent rester vivants le temps de l'appel (voir RenderSystem.cpp ou
//  on le construit sur la pile juste avant draw_mesh).
// ------------------------------------------------------------
struct drawItem
{
	const Mesh& mesh;                                     // geometrie : sommets, faces, uvs
	const std::vector<Maths::Vec3_SSE>& vertex_normals;   // une normale lissée par sommet (calculée dans Mesh.cpp)
	const Materials& mat;                                 // couleur diffuse + texture + transparence
	const Maths::Matrix4x4_SSE& model;                    // local -> monde (sert a tourner les normales et a connaitre la position de l'objet)
	const Maths::Matrix4x4_SSE& mvp;                      // projection * vue * model, local -> clip space direct
	const Maths::Vec3_SSE& light;                         // direction de la lumiere (pour le diffus Lambert)
	const Maths::Vec3_SSE& half_vec;                      // vecteur "half" (pour le speculaire Blinn-Phong)
};

// ------------------------------------------------------------
//  Renderer
//  possede framebuffer + zbuffer + rasterizer et enchaine les etapes du pipeline :
//  clipping near -> projection écran -> back-face culling -> rasterisation
// ------------------------------------------------------------
class Renderer
{
private:
	// L'ordre compte : le Rasterizer référence _fb et _zb
	//  _fb et _zb doivent etre declarés avant _rtz sinon
	//  le rasterizer garderait une reference sur un objet pas encore construit
	FrameBuffer _fb;
	ZBuffer _zb;
	Rasterizer _rtz;

	std::vector<ScreenTri> _tris;          // tous les triangles de la frame, deja projetés en coordonnées ecran
	std::vector<std::vector<int>> _bins;   // un bin par tuile : indices dans _tris
	int _tilesX, _tilesY;                  // nombre de tuiles en largeur / hauteur
	static constexpr int TILE = 64;        // taille d'une tuile en pixels (64x64)

public:
	// width/height = taille du framebuffer ET du zbuffer (en pixels, pas la taille de la fenetre)
	Renderer(int width, int height) : _fb(width, height), _zb(width, height), _rtz(_fb, _zb)
	{
		// division entiere arrondie au dessus : si l'ecran fait 700px on veut 11 tuiles (pas 10)
		// sinon les derniers pixels ne seraient dans aucune tuile
		_tilesX = (width + TILE - 1) / TILE;
		_tilesY = (height + TILE - 1) / TILE;
		_bins.resize(_tilesX * _tilesY);    // une liste vide par tuile, on la remplit a chaque frame
	}

	// acces au framebuffer, utilisé par main pour l'afficher (Window::Present) ou dessiner le texte par dessus
	FrameBuffer& framebuffer() { return _fb; }

	// fb.Clear + zb.clear_Zbuffer (profondeur remise à « infini »)
	// a appeler UNE fois au debut de chaque frame, avant tout draw_*
	// vide aussi la liste de triangles et les bins de la frame d'avant
	void begin_frame(uint32_t clear_color);

	// a appeler UNE fois a la fin de la frame, apres tous les draw_mesh :
	// c'est la que les triangles sont vraiment dessinés (binning + threads)
	void end_Frame();

	// dessine les particules (petits disques additifs) directement dans le framebuffer
	// view_proj = proj * view (les particules sont deja en coordonnées monde, pas de model)
	// focal_px  = focale en pixels, sert a calculer la taille a l'ecran d'une particule
	void draw_particles(const ParticleSystem& ps, const Maths::Matrix4x4_SSE& view_proj, float focal_px);

	// La boucle sur les faces qui était dans main.cpp
	// pour chaque face : mvp -> clipping near -> project_and_draw
	// writeDepth = false prevu pour les objets transparents (ne pas ecrire dans le zbuffer)
	void draw_mesh(const drawItem& item, bool writeDepth = true);

	// Ancienne ZBuffer::project_and_draw
	// prend un triangle deja clippé (en clip space), le projette a l'ecran,
	// fait le back-face culling et l'ajoute a _tris. appelé par draw_mesh
	void project_and_draw(const clip_vertex tri[3], const drawItem& item);

	// version "scene complete" : opaques d'abord, puis transparents triés du plus loin au plus proche
	// camera_pos sert pour le tri des transparents
	void draw_scene(const std::vector<drawItem>& scene, const Maths::Vec3_SSE& camera_pos);
};

// Position de l'objet en espace monde (extrait de la matrice de modèle ou position centrale)
// Si ton objet est centré en (0,0,0) local, sa position monde est la colonne de translation de la matrice model :
// (dans mon cas la translation est stockée dans rows[3] vu que la matrice est en convention "ligne",
//  c'est pour ca qu'on lit rows[3] et pas une colonne)
// utilisé dans draw_scene pour trier les objets transparents par distance a la camera
inline Maths::Vec3_SSE get_position(const Maths::Matrix4x4_SSE& m) {
	return Maths::Vec3_SSE(m.rows[3].m128_f32[0], m.rows[3].m128_f32[1], m.rows[3].m128_f32[2]);
}