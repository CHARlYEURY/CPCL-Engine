#pragma once
#include "Framebuffer.h"


class Rasterizer
{
private : 
	FrameBuffer& _mFrambuffer;
public : 

	Rasterizer(FrameBuffer& framebuffer) : _mFrambuffer(framebuffer) {};

	void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
	int edge(int ax, int ay, int bx, int by, int px, int py);
	uint32_t shade(int e0, int e1, int e2, int wsum, uint32_t c0, uint32_t c1, uint32_t c2);
};