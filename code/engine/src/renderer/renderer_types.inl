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

typedef struct geometry_render_data
{
    mat4 Model;
    geometry* Geometry;
} geometry_render_data;

typedef enum builtin_renderpass
{
    BUILTIN_RENDERPASS_WORLD = 0x01,
    BUILTIN_RENDERPASS_UI    = 0x02,
} builtin_renderpass;

typedef struct renderer_backend
{
    struct platform_state* PlatState;
    u32 FrameNumber;

    b8 (*Initialize)(struct renderer_backend* Backend, const char* ApplicationName);
    void (*Shutdown)(struct renderer_backend* Backend);
    void (*Resized)(struct renderer_backend* Backend, u16 Width, u16 Height);

    b8 (*BeginFrame)(struct renderer_backend* Backend, r32 DeltaTime);
    void (*UpdateGlobalWorldState)(mat4 Projection, mat4 View, v3 ViewPosition, v4 AmbientColor, s32 Mode);
    void (*UpdateGlobalUiState)(mat4 Projection, mat4 View, s32 Mode);
    void (*DrawGeometry)(geometry_render_data RenderData);
    b8 (*EndFrame)(struct renderer_backend* Backend, r32 DeltaTime);

    b8 (*BeginRenderpass)(struct renderer_backend* Backend, u8 RenderpassID);
    b8 (*EndRenderpass)(struct renderer_backend* Backend, u8 RenderpassID);

    void (*CreateTexture)(const u8* Pixels, texture* Texture);
    void (*DestroyTexture)(texture* Texture);

    b8 (*CreateMaterial)(material* Material);
    void (*DestroyMaterial)(material* Material);

    b8 (*CreateGeometry)(geometry* Geometry, u32 VertexSize, u32 VertexCount, const void* Vertices, u32 IndexSize, u32 IndexCount, const void* Indices);
    void (*DestroyGeometry)(geometry* Geometry);
} renderer_backend;

typedef struct render_packet
{
    r32 DeltaTime;

    u32 GeometryCount;
    geometry_render_data* Geometries;

    u32 UiGeometryCount;
    geometry_render_data* UiGeometries;
} render_packet;

