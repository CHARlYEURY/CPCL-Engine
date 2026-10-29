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


struct drawItem
{
	const Mesh& mesh;
	const std::vector<Maths::Vec3_SSE>& vertex_normals;
	const Materials& mat;
	const Maths::Matrix4x4_SSE& model;
	const Maths::Matrix4x4_SSE& mvp;
	const Maths::Vec3_SSE& light;
	const Maths::Vec3_SSE& half_vec;
};

// Possède framebuffer + zbuffer + rasterizer et enchaîne les étapes du pipeline :
// clipping near -> projection écran -> back-face culling -> rasterisation
class Renderer
{
private:
	// L'ordre compte : le Rasterizer référence _fb et _zb
	FrameBuffer _fb;
	ZBuffer _zb;
	Rasterizer _rtz;

	std::vector<ScreenTri> _tris;
	std::vector<std::vector<int>> _bins;   // un bin par tuile : indices dans _tris
	int _tilesX, _tilesY;
	static constexpr int TILE = 64;

public:
	Renderer(int width, int height) : _fb(width, height), _zb(width, height), _rtz(_fb, _zb) 
	{
		_tilesX = (width + TILE - 1) / TILE;
		_tilesY = (height + TILE - 1) / TILE;
		_bins.resize(_tilesX * _tilesY);
	}

	FrameBuffer& framebuffer() { return _fb; }

	// fb.Clear + zb.clear_Zbuffer (profondeur remise à « infini »)
	void begin_frame(uint32_t clear_color);
	void end_Frame();

	void draw_particles(const ParticleSystem& ps, const Maths::Matrix4x4_SSE& view_proj, float focal_px);

	// La boucle sur les faces qui était dans main.cpp
	void draw_mesh(const drawItem& item, bool writeDepth = true);

	// Ancienne ZBuffer::project_and_draw
	void project_and_draw(const clip_vertex tri[3], const drawItem& item);

	void draw_scene(const std::vector<drawItem>& scene, const Maths::Vec3_SSE& camera_pos);
};

// Position de l'objet en espace monde (extrait de la matrice de modèle ou position centrale)
	// Si ton objet est centré en (0,0,0) local, sa position monde est la colonne de translation de la matrice model :
inline Maths::Vec3_SSE get_position(const Maths::Matrix4x4_SSE& m) {
	return Maths::Vec3_SSE(m.rows[3].m128_f32[0], m.rows[3].m128_f32[1], m.rows[3].m128_f32[2]);
}