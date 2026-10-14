#include "Framebuffer.h"
#include <algorithm>

#include <fstream>

FrameBuffer::FrameBuffer(int width, int height) : size_width(width), size_height(height), pixels(size_t(width)* height, 0)
{

}

void FrameBuffer::Clear(uint32_t color)
{
    std::fill(pixels.begin(), pixels.end(), color);
}

void FrameBuffer::SetPixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= size_width || y < 0 || y >= size_height)
        return;
    pixels[size_t(y) * size_width + x] = color;
}

int FrameBuffer::getWidth() const
{
    return size_width;
}

int FrameBuffer::getHeight() const
{
    return size_height;
}

const uint32_t* FrameBuffer::Data() const
{
    return pixels.data();
}

bool FrameBuffer::SavePPM(const std::string& path) const
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;

    file << "P6\n" << size_width << " " << size_height << "\n255\n";
    for (uint32_t p : pixels)
    {
        const char rgb[3] = {
            char((p >> 16) & 0xFF),
            char((p >> 8) & 0xFF),
            char(p & 0xFF)
        };
        file.write(rgb, 3);
    }
    return true;
}