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

b8 BeginFrame(r32 DeltaTime)
{
    if(!RendererState)
    {
        return false;
    }

    return RendererState->Backend.BeginFrame(&RendererState->Backend, DeltaTime);
}

b8 EndFrame(r32 DeltaTime)
{
    if(!RendererState)
    {
        return false;
    }

    b8 Result = RendererState->Backend.EndFrame(&RendererState->Backend, DeltaTime);
    RendererState->Backend.FrameNumber++;
    return Result;
}

void RendererOnResize(u16 Width, u16 Height)
{
    if(RendererState)
    {
        RendererState->Projection = Perspective(DegToRad(45.0f), (r32)Width / (r32)Height, RendererState->NearClip, RendererState->FarClip);
        RendererState->Backend.Resized(&RendererState->Backend, Width, Height);
    }
    else
    {
        VENG_WARN("Renderer backend does not exist to accept resize");
    }
}

void RendererSetView(mat4* View)
{
    RendererState->View = *View;
}

b8 RendererDrawFrame(render_packet* Packet)
{
    if(BeginFrame(Packet->DeltaTime))
    {
        RendererState->Backend.UpdateGlobalState(RendererState->Projection, RendererState->View, V3Zero(), V4One(), 0);

        u32 Count = Packet->GeometryCount;
        for(u32 GeometryIndex = 0;
            GeometryIndex < Count;
            ++GeometryIndex)
        {
            RendererState->Backend.DrawGeometry(Packet->Geometries[GeometryIndex]);
        }

        b8 Result = EndFrame(Packet->DeltaTime);

        if(!Result)
        {
            VENG_FATAL("RendererEndFrame failed. Application shutting down");
            return false;
        }
    }

    return true;
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

b8 RendererCreateGeometry(geometry *Geometry, u32 VertexCount, const vertex_3d *Vertices, u32 IndexCount, const u32* Indices)
{
    return RendererState->Backend.CreateGeometry(Geometry, VertexCount, Vertices, IndexCount, Indices);
}

void RendererDestroyGeometry(geometry* Geometry)
{
    RendererState->Backend.DestroyGeometry(Geometry);
}

