#include "vmath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 RandSeeded = false;

VENG_API r32 Sin(r32 X)
{
    return sinf(X);
}

VENG_API r32 Cos(r32 X)
{
    return cosf(X);
}

VENG_API r32 ArcCos(r32 X)
{
    return acosf(X);
}

VENG_API r32 Tan(r32 X)
{
    return tanf(X);
}

VENG_API r32 Atan(r32 X)
{
    return atanf(X);
}

VENG_API r32 SquareRoot(r32 X)
{
    return sqrtf(X);
}

VENG_API r32 Abs(r32 X)
{
    return fabsf(X);
}

VENG_API s32 Random()
{
    if(!RandSeeded)
    {
        srand((u32)PlatformGetAbsoluteTime());
        RandSeeded;
    }

    return rand();
}

VENG_API s32 RanfomInRange(s32 Min, s32 Max)
{
    if(!RandSeeded)
    {
        srand((u32)PlatformGetAbsoluteTime());
        RandSeeded;
    }

    return (rand() % (Max - Min + 1)) + Min;
}

VENG_API r32 FloatRandom()
{
    return (r32)Random() / (r32)RAND_MAX;
}

VENG_API r32 FloatRandomInRange(s32 Min, s32 Max)
{
    return Min + ((r32)Random() / ((r32)RAND_MAX / (Max - Min)));
}

