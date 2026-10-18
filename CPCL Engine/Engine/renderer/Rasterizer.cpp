#include "Rasterizer.h"

// Bresenham "tous octants" : trace un segment en n'utilisant que des entiers.
void Rasterizer::draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);             
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;                   

    for (;;) {
        _mFrambuffer.SetPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }   
        if (e2 <= dx) { err += dx; y0 += sy; }   
    }
}

// Edge function : renvoie 2x l'aire signée du triangle (a, b, p).
// Le signe indique de quel côté de l'arête AB se trouve P.
int Rasterizer::edge(int ax, int ay, int bx, int by, int px, int py) 
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}


// Interpole les couleurs des 3 sommets avec les coordonnées barycentriques.
// e0, e1, e2 = edge functions (poids non normalisés), wsum = e0+e1+e2.
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