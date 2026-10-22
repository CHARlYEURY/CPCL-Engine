#pragma once
#include <vector>

// Ne contient plus que le buffer de profondeur.
// Le rasterisation est dans Rasterizer, l'éclairage dans Lighting, les normales dans Mesh.
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

	// Même principe que FrameBuffer::operator[]
	float& operator[](size_t idx) { return zbuffer[idx]; }
	const float& operator[](size_t idx) const { return zbuffer[idx]; }

	int getWidth() const { return Widths; }
	int getHeight() const { return Heights; }

	void clear_Zbuffer();
};
