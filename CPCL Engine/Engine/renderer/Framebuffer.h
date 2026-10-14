#pragma once
#include <vector>
#include <string>

constexpr uint32_t PackColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
	return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

class FrameBuffer
{
private:

	int size_width, size_height;
	std::vector <uint32_t> pixels;
public:

	FrameBuffer(int width, int height);

	void Clear(uint32_t color);
	void SetPixel(int x, int y, uint32_t color);

	int getWidth() const;
	int getHeight() const;
	const uint32_t* Data() const;

	// Export pour vérifier le rendu sans fenêtre
	bool SavePPM(const std::string& path) const;
};