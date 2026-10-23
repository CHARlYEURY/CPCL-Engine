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
#include <atomic>
#include <thread>
#include <algorithm>

struct ScreenTri {
	int x[3], y[3];
	float z[3], iw[3];
	Maths::UV uv[3];
	Maths::Vec3_SSE n[3];
	const Texture* tex;
	Maths::Vec3_SSE L, H;
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

	// La boucle sur les faces qui était dans main.cpp
	void draw_mesh(const Mesh& mesh,
		const std::vector<Maths::Vec3_SSE>& vertex_normals,
		const Texture& tex,
		const Maths::Matrix4x4_SSE& model,
		const Maths::Matrix4x4_SSE& mvp,
		const Maths::Vec3_SSE& light,
		const Maths::Vec3_SSE& half_vec);

	// Ancienne ZBuffer::project_and_draw
	void project_and_draw(const clip_vertex tri[3],
		const Texture& tex,
		const Maths::Vec3_SSE& L,
		const Maths::Vec3_SSE& H);
};
