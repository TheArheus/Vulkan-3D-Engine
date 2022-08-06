#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "math/vmath.h"

#include "core/vstring.h"
#include "core/event.h"

#include "resources/resource_types.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"

typedef struct renderer_state
{
    renderer_backend Backend;

    mat4 Projection;
    mat4 View;

    mat4 UiProjection;
    mat4 UiView;

    r32 NearClip;
    r32 FarClip;
} renderer_state;

static renderer_state* RendererState;

b8 RendererInitialize(u64* MemoryRequirement, void* State, const char* ApplicationName)
{
    *MemoryRequirement = sizeof(renderer_state);
    if(State == 0)
    {
        return true;
    }

    RendererState = State;

    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &RendererState->Backend);
    RendererState->Backend.FrameNumber = 0;

    if(!RendererState->Backend.Initialize(&RendererState->Backend, ApplicationName))
    {
        VENG_FATAL("Renderer backend failed to initialize. Shutting down");
        return false;
    }

    RendererState->NearClip = 0.1f;
    RendererState->FarClip  = 1000.0f;
    RendererState->Projection = Perspective(DegToRad(45.0f), 1280.0f/720.0f, RendererState->NearClip, RendererState->FarClip);
    RendererState->View = Translation(V3(0, 0, -30.0f));
    RendererState->View = Inverse(RendererState->View);

    RendererState->UiProjection = Orthographic(0, 1280.0f, 720.0f, 0, -100.0f, 100.0f);
    RendererState->UiView = Inverse(Identity());

    return true;
}

void RendererShutdown(void* State)
{
    if(RendererState)
    {
        RendererState->Backend.Shutdown(&RendererState->Backend);
    }

    RendererState = 0;
}

void RendererOnResize(u16 Width, u16 Height)
{
    if(RendererState)
    {
        RendererState->Projection = Perspective(DegToRad(45.0f), (r32)Width / (r32)Height, RendererState->NearClip, RendererState->FarClip);
        RendererState->UiProjection = Orthographic(0, (r32)Width, (r32)Height, 0, -100.0f, 100.0f);
        RendererState->Backend.Resized(&RendererState->Backend, Width, Height);
    }
    else
    {
        VENG_WARN("Renderer backend does not exist to accept resize");
    }
}

b8 RendererDrawFrame(render_packet* Packet)
{
    if(RendererState->Backend.BeginFrame(&RendererState->Backend, Packet->DeltaTime))
    {
        // NOTE: World Renderpass
        if(!RendererState->Backend.BeginRenderpass(&RendererState->Backend, BUILTIN_RENDERPASS_WORLD))
        {
            VENG_ERROR("BeginRenderpass - BUILTIN_RENDERPASS_WORLD failed. Application shutting down...");
            return false;
        }

        RendererState->Backend.UpdateGlobalWorldState(RendererState->Projection, RendererState->View, V3Zero(), V4One(), 0);

        u32 Count = Packet->GeometryCount;
        for(u32 GeometryIndex = 0;
            GeometryIndex < Count;
            ++GeometryIndex)
        {
            RendererState->Backend.DrawGeometry(Packet->Geometries[GeometryIndex]);
        }

        if(!RendererState->Backend.EndRenderpass(&RendererState->Backend, BUILTIN_RENDERPASS_WORLD))
        {
            VENG_ERROR("EndRenderpass - BUILTIN_RENDERPASS_WORLD failed. Application shutting down...");
            return false;
        }


        // NOTE: Ui Renderpass
        if(!RendererState->Backend.BeginRenderpass(&RendererState->Backend, BUILTIN_RENDERPASS_UI))
        {
            VENG_ERROR("BeginRenderpass - BUILTIN_RENDERPASS_UI failed. Application shutting down...");
            return false;
        }

        RendererState->Backend.UpdateGlobalUiState(RendererState->UiProjection, RendererState->UiView, 0);

        u32 UiCount = Packet->UiGeometryCount;
        for(u32 GeometryIndex = 0;
            GeometryIndex < UiCount;
            ++GeometryIndex)
        {
            RendererState->Backend.DrawGeometry(Packet->UiGeometries[GeometryIndex]);
        }

        if(!RendererState->Backend.EndRenderpass(&RendererState->Backend, BUILTIN_RENDERPASS_UI))
        {
            VENG_ERROR("EndRenderpass - BUILTIN_RENDERPASS_UI failed. Application shutting down...");
            return false;
        }

        b8 Result = RendererState->Backend.EndFrame(&RendererState->Backend, Packet->DeltaTime);
        RendererState->Backend.FrameNumber++;

        if(!Result)
        {
            VENG_FATAL("RendererEndFrame failed. Application shutting down");
            return false;
        }
    }

    return true;
}

void RendererSetView(mat4* View)
{
    RendererState->View = *View;
}

void RendererCreateTexture(const u8* Pixels, texture* Texture)
{
    RendererState->Backend.CreateTexture(Pixels, Texture);
}

void RendererDestroyTexture(texture* Texture)
{
    RendererState->Backend.DestroyTexture(Texture);
}


b8 RendererCreateMaterial(material* Material)
{
    return RendererState->Backend.CreateMaterial(Material);
}

void RendererDestroyMaterial(material* Material)
{
    RendererState->Backend.DestroyMaterial(Material);
}

b8 RendererCreateGeometry(geometry* Geometry, u32 VertexSize, u32 VertexCount, const void* Vertices, u32 IndexSize, u32 IndexCount, const void* Indices)
{
    return RendererState->Backend.CreateGeometry(Geometry, VertexSize, VertexCount, Vertices, IndexSize, IndexCount, Indices);
}

void RendererDestroyGeometry(geometry* Geometry)
{
    RendererState->Backend.DestroyGeometry(Geometry);
}

