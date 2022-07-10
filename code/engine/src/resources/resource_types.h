#pragma once

#include "math/math_types.h"

typedef struct texture
{
    u32 ID;
    u32 Width;
    u32 Height;
    u8  ChannelCount;
    u8  HasTransparency;
    u32 Generation;

    void* Data;
} texture;
