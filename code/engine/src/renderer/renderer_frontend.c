#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "math/vmath.h"

#include "core/vstring.h"
#include "core/event.h"

#include "resources/resource_types.h"
#include "systems/texture_system.h"

typedef struct renderer_state
{
    renderer_backend Backend;
    mat4 Projection;
    mat4 View;
    r32 NearClip;
    r32 FarClip;

    texture* TestDiffuse;
} renderer_state;

static renderer_state* RendererState;

b8 EventOnDebugEvent(u16 Code, void* Sender, void* ListenerInst, event_context Data)
{
    const char* Names[3] = 
    {
        "cobblestone",
        "paving",
        "paving2"
    };
    static s8 choice = 2;
    const char* OldName = Names[choice];
    choice++;
    choice %= 3;

    RendererState->TestDiffuse = TextureSystemAcquire(Names[choice], true);
    TextureSystemRelease(OldName);

    return true;
}

b8 RendererInitialize(u64* MemoryRequirement, void* State, const char* ApplicationName)
{
    *MemoryRequirement = sizeof(renderer_state);
    if(State == 0)
    {
        return true;
    }

    RendererState = State;
    EventRegister(EVENT_CODE_DEBUG0, RendererState, EventOnDebugEvent);

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
        EventUnregister(EVENT_CODE_DEBUG0, RendererState, EventOnDebugEvent);
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

        static r32 Angle = 0.01f;
        Angle += 0.001f;
        quat Rotation = QuatFromAxis(V3Forward(), Angle, false);
        mat4 Model = QuatToRot(Rotation, V3Zero());

        geometry_render_data Data = {};
        Data.ObjectID = 0;
        Data.Model = Model;

        if(!RendererState->TestDiffuse)
        {
            RendererState->TestDiffuse = TextureSystemGetDefaultTexture();
        }

        Data.Textures[0] = RendererState->TestDiffuse;
        RendererState->Backend.UpdateObject(Data);

        b8 Result = EndFrame(Packet->DeltaTime);

        if(!Result)
        {
            VENG_FATAL("RendererEndFrame failed. Application shutting down");
            return false;
        }
    }

    return true;
}

void RendererCreateTexture(const char* Name, u32 Width, u32 Height, u32 ChannelCount, const u8* Pixels, b8 HasTransparency, texture* OutTexture)
{
    RendererState->Backend.CreateTexture(Name, Width, Height, ChannelCount, Pixels, HasTransparency, OutTexture);
}

void RendererDestroyTexture(texture* Texture)
{
    RendererState->Backend.DestroyTexture(Texture);
}


