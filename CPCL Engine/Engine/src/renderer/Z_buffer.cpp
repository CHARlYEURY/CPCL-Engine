#include "renderer/Z_buffer.h"

void ZBuffer::clear_Zbuffer()
{
    for (int i = 0; i < Widths * Heights; i++)
    {
        zbuffer[i] = 1e30f;
    }
}
