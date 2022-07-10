#pragma once

#include "defines.h"
#include "math/math_types.h"
#include "resources/resource_types.h"

typedef enum renderer_backend_type
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX,
} renderer_backend_type;

typedef struct global_uniform_object
{
    mat4 Projection;
    mat4 View;
    mat4 Reserved0;
    mat4 Reserved1;
} global_uniform_object;

// NOTE: global_uniform_object
typedef struct local_uniform_object 
{
    v4 DiffuseColor;
    v4 Reserved0;
    v4 Reserved1;
    v4 Reserved2;
} local_uniform_object;

typedef struct geometry_render_data
{
    u32  ObjectID;
    mat4 Model;
    texture* Textures[16];
} geometry_render_data;

typedef struct renderer_backend
{
    struct platform_state* PlatState;
    u32 FrameNumber;

    b8 (*Initialize)(struct renderer_backend* Backend, const char* ApplicationName);
    void (*Shutdown)(struct renderer_backend* Backend);
    void (*Resized)(struct renderer_backend* Backend, u16 Width, u16 Height);

    b8 (*BeginFrame)(struct renderer_backend* Backend, r32 DeltaTime);
    void (*UpdateGlobalState)(mat4 Projection, mat4 View, v3 ViewPosition, v4 AmbientColor, s32 Mode);
    void (*UpdateObject)(geometry_render_data RenderData);
    b8 (*EndFrame)(struct renderer_backend* Backend, r32 DeltaTime);

    void (*CreateTexture)(const char* Name, u32 Width, u32 Height, u32 ChannelCount, const u8* Pixels, b8 HasTransparency, texture* OutTexture);
    void (*DestroyTexture)(texture* Texture);
} renderer_backend;

typedef struct render_packet
{
    r32 DeltaTime;
} render_packet;

