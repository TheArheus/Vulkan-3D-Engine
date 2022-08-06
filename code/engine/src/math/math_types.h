#pragma once

#include "defines.h"

typedef union v2
{
    struct
    {
        union
        {
            r32 x, r, s, u;
        };
        union
        {
            r32 y, g, t, v;
        };
    };
    r32 E[2];
} v2;

typedef union v3
{
    struct
    {
        union
        {
            r32 x, r, s, u;
        };
        union
        {
            r32 y, g, t, v;
        };
        union
        {
            r32 z, b, p, w;
        };
    };
    r32 E[3];
} v3;

typedef union v4
{
    struct
    {
        union
        {
            r32 x, r, s;
        };
        union
        {
            r32 y, g, t;
        };
        union
        {
            r32 z, b, p;
        };
        union
        {
            r32 w, a, q;
        };
    };
    r32 E[4];
} v4;

typedef v4 quat;

typedef union mat4
{
    r32 E[16];
} mat4;

typedef struct vertex_2d
{
    v2 Position;
    v2 TexCoord;
} vertex_2d;

typedef struct vertex_3d
{
    v3 Position;
    v2 TexCoord;
} vertex_3d;

