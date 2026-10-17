#pragma once
#include "Framebuffer.h"
#include "Rasterizer.h"
#include <vector>

class ZBuffer
{
private:
	int Widths, Heights;
	std::vector<float> zbuffer;
public:

	ZBuffer(int width, int height)
		: Widths(width), Heights(height), zbuffer(width* height, 1e30f)
	{
	}


	void clear_Zbuffer();
	int clampi(int v, int lo, int hi);
	int edge(int ax, int ay, int bx, int by, int px, int py);
	void draw_triangle_zbuffer(
		int x0, int y0, float z0,
		int x1, int y1, float z1,
		int x2, int y2, float z2,
		float zmin, float zrange,
		Rasterizer& rtz, FrameBuffer& fb);
};