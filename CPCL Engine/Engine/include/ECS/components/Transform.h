#pragma once 
#include "Vec3_SSE.h"

struct Transform {
    Maths::Vec3_SSE position{ 0, 0, 0 };
    float rotX = 0, rotY = 0;      // simple pour commencer
    float scale = 1;
};