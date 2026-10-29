#pragma once
#include "renderer/Framebuffer.h"
#include "renderer/Z_buffer.h"
#include "asset/textureLoader.h"
#include "Vec3_SSE.h"
#include "asset/Materials.h"

struct TileRect { int x0, y0, x1, y1; };   // x1,y1 exclus

struct ScreenTri {
	int x[3], y[3];
	float z[3], iw[3];
	Maths::UV uv[3];
	Maths::Vec3_SSE n[3];
	const Materials* mat;
	Maths::Vec3_SSE L, H;
};

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
	void draw_triangle_phong(const ScreenTri& tri, const TileRect& r, bool writeDepth = true);
};
