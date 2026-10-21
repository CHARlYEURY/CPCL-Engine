#pragma once
#include "Framebuffer.h"
#include "Rasterizer.h"
#include "asset/textureLoader.h"
#include "asset/AssetLoader.h"
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
	float pow32(float x);
	uint32_t color_phong(uint32_t texel, float diffuse_k, float spec_k);
	uint32_t color_scale(uint32_t c, float k);
	void compute_vertex_normals(const Mesh& mesh, const std::vector<Maths::Vec3_SSE>& face_normals, std::vector<Maths::Vec3_SSE>& out);
	void compute_face_normals(const Mesh& mesh, std::vector<Maths::Vec3_SSE>& out);
	void draw_triangle_zbuffer(
		int x0, int y0, float z0, float iw0, Maths::UV uv0, Maths::Vec3_SSE n0,
		int x1, int y1, float z1, float iw1, Maths::UV uv1, Maths::Vec3_SSE n1,
		int x2, int y2, float z2, float iw2, Maths::UV uv2, Maths::Vec3_SSE n2, Rasterizer& rtz, FrameBuffer& fb, const Texture& tex, Maths::Vec3_SSE L, Maths::Vec3_SSE H);

	void project_and_draw(const clip_vertex* tri, const Texture* tex, vec3 L, vec3 H);
};