#pragma once
#include "renderer/Framebuffer.h"
#include "renderer/Z_buffer.h"
#include "asset/textureLoader.h"
#include "Vec3_SSE.h"
#include "asset/Materials.h"

class Rasterizer
{
private : 
	FrameBuffer& _mFramebuffer;
	ZBuffer& _mZBuffer;
public : 

	Rasterizer(FrameBuffer& framebuffer, ZBuffer& zbuffer) : _mFramebuffer(framebuffer), _mZBuffer(zbuffer) {};

	void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
	int edge(int ax, int ay, int bx, int by, int px, int py);
	uint32_t shade(int e0, int e1, int e2, int wsum, uint32_t c0, uint32_t c1, uint32_t c2);

	// rasterise un triangle avec Z-test, texture perspective-correcte et Phong
	// (framebuffer et zbuffer sont maintenant des membres : plus besoin de les passer en paramètre)
	void draw_triangle_phong(
		int x0, int y0, float z0, float iw0, Maths::UV uv0, Maths::Vec3_SSE n0,
		int x1, int y1, float z1, float iw1, Maths::UV uv1, Maths::Vec3_SSE n1,
		int x2, int y2, float z2, float iw2, Maths::UV uv2, Maths::Vec3_SSE n2, const Materials& mat, Maths::Vec3_SSE L, Maths::Vec3_SSE H, bool writeDepth = true);
};
