// ============================================================
//  Z_buffer.cpp
// ------------------------------------------------------------
#include "renderer/Z_buffer.h"

// ------------------------------------------------------------
//  clear_Zbuffer : remet la profondeur de chaque pixel a "infini" (1e30f)
//  a faire a chaque frame, sinon les profondeurs de la frame d'avant
//  bloqueraient le dessin (un objet qui bouge serait rejeté par le z-test)
//  appelée par Renderer::begin_frame, en meme temps que FrameBuffer::Clear
// ------------------------------------------------------------
void ZBuffer::clear_Zbuffer()
{
    for (int i = 0; i < Widths * Heights; i++)
    {
        zbuffer[i] = 1e30f;
    }
}