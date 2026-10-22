#pragma once
#include <cstdint>
#include <vector>
#include <string>

constexpr uint32_t setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
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
	//FrameBuffer(int mul);

	// Accès en écriture
	uint32_t& operator[](size_t idx) { return pixels[idx]; }
	// Accès en lecture (objets const)
	const uint32_t& operator[](size_t idx) const { return pixels[idx]; }


	void Clear(uint32_t color);
	void SetPixel(int x, int y, uint32_t color);

	int getWidth() const;
	int getHeight() const;
	const uint32_t* Data() const;
	uint32_t* Data();

	// Export pour vérifier le rendu sans fenêtre
	bool SavePPM(const std::string& path) const;
};
