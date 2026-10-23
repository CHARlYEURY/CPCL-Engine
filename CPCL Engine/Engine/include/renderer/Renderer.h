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
#include "asset/Materials.h"

// Possède framebuffer + zbuffer + rasterizer et enchaîne les étapes du pipeline :
// clipping near -> projection écran -> back-face culling -> rasterisation
class Renderer
{
private:
	// L'ordre compte : le Rasterizer référence _fb et _zb
	FrameBuffer _fb;
	ZBuffer _zb;
	Rasterizer _rtz;

public:
	Renderer(int width, int height) : _fb(width, height), _zb(width, height), _rtz(_fb, _zb) {}

	FrameBuffer& framebuffer() { return _fb; }

	// fb.Clear + zb.clear_Zbuffer (profondeur remise à « infini »)
	void begin_frame(uint32_t clear_color);

	// La boucle sur les faces qui était dans main.cpp
	void draw_mesh(const Mesh& mesh,
		const std::vector<Maths::Vec3_SSE>& vertex_normals,
		const Materials& mat,
		const Maths::Matrix4x4_SSE& model,
		const Maths::Matrix4x4_SSE& mvp,
		const Maths::Vec3_SSE& light,
		const Maths::Vec3_SSE& half_vec);

	// Ancienne ZBuffer::project_and_draw
	void project_and_draw(const clip_vertex tri[3],
		const Materials& mat,
		const Maths::Vec3_SSE& L,
		const Maths::Vec3_SSE& H);
};
