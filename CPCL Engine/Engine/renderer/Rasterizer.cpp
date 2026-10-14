#include "Rasterizer.h"


void Rasterizer::draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);             // negative on purpose
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;                   // = dx - |dy|0

    for (;;) {
        _mFrambuffer.SetPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }   // step in x
        if (e2 <= dx) { err += dx; y0 += sy; }   // step in y
    }
}

int Rasterizer::edge(int ax, int ay, int bx, int by, int px, int py) 
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

uint32_t Rasterizer::shade(int e0, int e1, int e2, int wsum,
    uint32_t c0, uint32_t c1, uint32_t c2) {

    uint8_t r = (uint8_t)((e0 * ((c0 >> 16) & 0xFF)
        + e1 * ((c1 >> 16) & 0xFF)
        + e2 * ((c2 >> 16) & 0xFF)) / wsum);
    uint8_t g = (uint8_t)((e0 * ((c0 >> 8) & 0xFF)
        + e1 * ((c1 >> 8) & 0xFF)
        + e2 * ((c2 >> 8) & 0xFF)) / wsum);
    uint8_t b = (uint8_t)((e0 * (c0 & 0xFF)
        + e1 * (c1 & 0xFF)
        + e2 * (c2 & 0xFF)) / wsum);
    return setRGB(r, g, b);
}