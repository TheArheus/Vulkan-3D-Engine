#include "application.h"
#include "game_types.h"

#include "logger.h"

#include "platform/platform.h"
#include "core/vmemory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/vstring.h"
#include "clock.h"

#include "math/vmath.h"

#include "memory/linear_allocator.h"

#include "renderer/renderer_frontend.h"

#include "systems/texture_system.h"
#include "systems/material_system.h"
#include "systems/geometry_system.h"
#include "systems/resource_system.h"

typedef struct application_state
{
    game* GameInst;
    b8 IsRunning;
    b8 IsSuspended;
    s16 Width;
    s16 Height;
    clock Clock;
    r64 LastTime;
    linear_allocator SystemsAllocator;

    u64 EventSystemMemoryRequirement;
    void* EventSystem;

    u64 MemorySystemMemoryRequirement;
    void* MemorySystem;

    u64 LoggingSystemMemoryRequirement;
    void* LoggingSystem;

    u64 InputSystemMemoryRequirement;
    void* InputSystem;

    u64 PlatformSystemMemoryRequirement;
    void* PlatformSystem;

    u64 RendererSystemMemoryRequirement;
    void* RendererSystem;

    u64 TextureSystemMemoryRequirement;
    void* TextureSystem;

    u64 MaterialSystemMemoryRequirement;
    void* MaterialSystem;

    u64 GeometrySystemMemoryRequirement;
    void* GeometrySystem;

    u64 ResourceSystemMemoryRequirement;
    void* ResourceSystem;

    geometry* TestGeometry;
} application_state;

static application_state* AppState;

b8 EventOnDebugEvent(u16 code, void* sender, void* Listener, event_context Data) {
    const char* Names[3] = {
        "cobblestone",
        "paving",
        "paving2"};
    static s8 Choice = 2;

    // Save off the old name.
    const char* OldName = Names[Choice];

    Choice++;
    Choice %= 3;

    // Acquire the new texture.
    if (AppState->TestGeometry) {
        AppState->TestGeometry->Material->DiffuseMap.Texture = TextureSystemAcquire(Names[Choice], true);
        if (!AppState->TestGeometry->Material->DiffuseMap.Texture) {
            AppState->TestGeometry->Material->DiffuseMap.Texture = TextureSystemGetDefaultTexture();
        }

        // Release the old texture.
        TextureSystemRelease(OldName);
    }

    return true;
}

b8 ApplicationOnEvent(u16 Code, void* Sender, void* Listener, event_context Context);
b8 ApplicationOnResize(u16 Code, void* Sender, void* Listener, event_context Context);
b8 ApplicationOnKey(u16 Code, void* Sender, void* Listener, event_context Context);

b8 ApplicationCreate(game* GameInst)
{
    if(GameInst->ApplicationState)
    {
        VENG_ERROR("ApplicationCreate called more than one time");
        return false;
    }

    GameInst->ApplicationState = Allocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
    AppState = GameInst->ApplicationState;

    AppState->GameInst    = GameInst;
    AppState->IsRunning   = false;
    AppState->IsSuspended = false;

    u64 SystemsAllocatorTotalSize = 128*1024*1024;
    LinearAllocatorCreate(SystemsAllocatorTotalSize, 0, &AppState->SystemsAllocator);

    EventInitialize(&AppState->EventSystemMemoryRequirement, 0);
    AppState->EventSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->EventSystemMemoryRequirement);
    EventInitialize(&AppState->EventSystemMemoryRequirement, AppState->EventSystem);

    InitializeMemory(&AppState->MemorySystemMemoryRequirement, 0);
    AppState->MemorySystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->MemorySystemMemoryRequirement);
    InitializeMemory(&AppState->MemorySystemMemoryRequirement, AppState->MemorySystem);

    InitializeLogging(&AppState->LoggingSystemMemoryRequirement, 0);
    AppState->LoggingSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->LoggingSystemMemoryRequirement);
    if(!InitializeLogging(&AppState->LoggingSystemMemoryRequirement, AppState->LoggingSystem))
    {
        VENG_ERROR("Failed to initialize logging system. Shutting down");
        return false;
    }

    InitializeInput(&AppState->InputSystemMemoryRequirement, 0);
    AppState->InputSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->InputSystemMemoryRequirement);
    InitializeInput(&AppState->InputSystemMemoryRequirement, AppState->InputSystem);

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_RESIZED, 0, ApplicationOnResize);
    EventRegister(EVENT_CODE_DEBUG0, 0, EventOnDebugEvent);

    PlatformStartup(&AppState->PlatformSystemMemoryRequirement, 0, 0, 0, 0, 0, 0);
    AppState->PlatformSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->PlatformSystemMemoryRequirement);
    if(!PlatformStartup(&AppState->PlatformSystemMemoryRequirement, AppState->PlatformSystem, 
                        GameInst->AppConfig.Name, 
                        GameInst->AppConfig.StartX    , GameInst->AppConfig.StartY, 
                        GameInst->AppConfig.StartWidth, GameInst->AppConfig.StartHeight))
    {
        return false;
    }

    resource_system_config ResourceSysConfig;
    ResourceSysConfig.MaxLoaderCount = 32;
    ResourceSysConfig.AssetBasePath = "../assets";
    ResourceSystemInitialize(&AppState->ResourceSystemMemoryRequirement, 0, ResourceSysConfig);
    AppState->ResourceSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->ResourceSystemMemoryRequirement);
    if(!ResourceSystemInitialize(&AppState->ResourceSystemMemoryRequirement, AppState->ResourceSystem, ResourceSysConfig))
    {
        VENG_FATAL("Failed to initialize resource system. Aborting application");
        return false;
    }

    RendererInitialize(&AppState->RendererSystemMemoryRequirement, 0, 0);
    AppState->RendererSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->RendererSystemMemoryRequirement);
    if(!RendererInitialize(&AppState->RendererSystemMemoryRequirement, AppState->RendererSystem, GameInst->AppConfig.Name))
    {
        VENG_FATAL("Failed to initialize renderer. Aborting application");
        return false;
    }

    texture_system_config TextureSysConfig;
    TextureSysConfig.MaxTextureCount = 65536;
    TextureSystemInitialize(&AppState->TextureSystemMemoryRequirement, 0, TextureSysConfig);
    AppState->TextureSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->TextureSystemMemoryRequirement);
    if(!TextureSystemInitialize(&AppState->TextureSystemMemoryRequirement, AppState->TextureSystem, TextureSysConfig))
    {
        VENG_FATAL("Failed to initialize texture system. Aborting application");
        return false;
    }

    material_system_config MaterialSysConfig;
    MaterialSysConfig.MaxMaterialCount = 4096;
    MaterialSystemInitialize(&AppState->MaterialSystemMemoryRequirement, 0, MaterialSysConfig);
    AppState->MaterialSystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->TextureSystemMemoryRequirement);
    if(!MaterialSystemInitialize(&AppState->MaterialSystemMemoryRequirement, AppState->MaterialSystem, MaterialSysConfig))
    {
        VENG_FATAL("Failed to initialize material system. Aborting application");
        return false;
    }

    geometry_system_config GeometrySysConfig;
    GeometrySysConfig.MaxGeometryCount = 4096;
    GeometrySystemInitialize(&AppState->GeometrySystemMemoryRequirement, 0, GeometrySysConfig);
    AppState->GeometrySystem = LinearAllocatorAllocate(&AppState->SystemsAllocator, AppState->GeometrySystemMemoryRequirement);
    if(!GeometrySystemInitialize(&AppState->GeometrySystemMemoryRequirement, AppState->GeometrySystem, GeometrySysConfig))
    {
        VENG_FATAL("Failed to initialize geometry system. Aborting application");
        return false;
    }

    geometry_config Config = GeometrySystemGeneratePlaneConfig(10.0f, 5.0f, 5, 5, 5.0f, 2.0f, "test geometry", "test_material");
    AppState->TestGeometry = GeometrySystemAcquireFromConfig(Config, true);

    Free(Config.Vertices, sizeof(vertex_3d) * Config.VertexCount, MEMORY_TAG_ARRAY);
    Free(Config.Indices, sizeof(u32) * Config.IndexCount, MEMORY_TAG_ARRAY);

    if(!AppState->GameInst->Initialize(AppState->GameInst))
    {
        VENG_FATAL("Game failed to initialize");
        return false;
    }

    AppState->GameInst->OnResize(AppState->GameInst, AppState->Width, AppState->Height);

    return true;
}

b8 ApplicationRun()
{
    ClockStart(&AppState->Clock);
    ClockUpdate(&AppState->Clock);
    AppState->LastTime = AppState->Clock.Elapsed;
    AppState->IsRunning = true;

    r64 RunningTime = 0;
    u8 FrameCount = 0;
    r64 TargetFrameSeconds = 1.0f / 60;

    while(AppState->IsRunning)
    {
        if(!PlatformPumpMessages())
        {
            AppState->IsRunning = false;
        }

        if(!AppState->IsSuspended)
        {
            ClockUpdate(&AppState->Clock);
            r64 CurrentTime = AppState->Clock.Elapsed;
            r64 Delta = (CurrentTime - AppState->LastTime);
            r64 FrameStartTime = PlatformGetAbsoluteTime();

            if(!AppState->GameInst->Update(AppState->GameInst, (r32)Delta))
            {
                VENG_FATAL("Game update failed.");
                AppState->IsRunning = false;
                break;
            }

            if(!AppState->GameInst->Render(AppState->GameInst, (r32)Delta))
            {
                VENG_FATAL("Game render failed.");
                AppState->IsRunning = false;
                break;
            }

            render_packet Packet;
            Packet.DeltaTime = Delta;

            geometry_render_data TestRender;
            TestRender.Geometry = AppState->TestGeometry;
            TestRender.Model = Identity();

            Packet.GeometryCount = 1;
            Packet.Geometries = &TestRender;
            RendererDrawFrame(&Packet);

            r64 FrameEndTime = PlatformGetAbsoluteTime();
            r64 FrameElapsedTime = FrameEndTime - FrameStartTime;
            RunningTime += FrameElapsedTime;
            r64 RemainingSeconds = TargetFrameSeconds - FrameElapsedTime;

            if(RemainingSeconds > 0)
            {
                u64 RemainingMs = (RemainingSeconds * 1000);

                b8 LimitFrames = false;
                if(RemainingMs > 0 && LimitFrames)
                {
                    PlatformSleep(RemainingMs);
                }

                FrameCount++;
            }

            InputUpdate(Delta);

            AppState->LastTime = CurrentTime;
        }
    }

    AppState->IsRunning = false;

    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_RESIZED, 0, ApplicationOnResize);
    EventUnregister(EVENT_CODE_DEBUG0, 0, EventOnDebugEvent);

    InputShutdown(AppState->InputSystem);
    ResourceSystemShutdown(AppState->ResourceSystem);
    GeometrySystemShutdown(AppState->GeometrySystem);
    MaterialSystemShutdown(AppState->MaterialSystem);
    TextureSystemShutdown(AppState->TextureSystem);
    EventShutdown(AppState->EventSystem);
    RendererShutdown(AppState->RendererSystem);
    PlatformShutdown(AppState->PlatformSystem);
    ShutdownMemory(AppState->MemorySystem);

    return true;
}

void ApplicationGetFramebufferSize(u32* Width, u32* Height)
{
    *Width  = AppState->Width;
    *Height = AppState->Height;
}

b8 ApplicationOnEvent(u16 Code, void* Sender, void* Listener, event_context Context)
{
    switch(Code)
    {
        case EVENT_CODE_APPLICATION_QUIT:
        {
            VENG_INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            AppState->IsRunning = false;
            return true;
        } break;
    }

    return false;
}

b8 ApplicationOnKey(u16 Code, void* Sender, void* Listener, event_context Context)
{
    if(Code == EVENT_CODE_KEY_PRESSED)
    {
        u16 KeyCode = Context.data.Unsigned16[0];
        if(KeyCode == KEY_ESCAPE)
        {
            event_context Data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, Data);

            return true;
        }
        else if(KeyCode == KEY_A)
        {
            VENG_DEBUG("Key A is pressed");
        }
        else
        {
            VENG_DEBUG("'%c' key pressed in window.", KeyCode);
        }
    }
    else if(Code == EVENT_CODE_KEY_RELEASED)
    {
        u16 KeyCode = Context.data.Unsigned16[0];
        if(KeyCode == KEY_B)
        {
            VENG_DEBUG("Explicit - B key released.");
        }
        else
        {
            VENG_DEBUG("'%c' key released in window.", KeyCode);
        }
    }

    return false;
}

b8 ApplicationOnResize(u16 Code, void* Sender, void* Listener, event_context Context)
{
    if(Code == EVENT_CODE_RESIZED)
    {
        u16 Width  = Context.data.Unsigned16[0];
        u16 Height = Context.data.Unsigned16[1];

        if(Width != AppState->Width || Height != AppState->Height)
        {
            AppState->Width  = Width;
            AppState->Height = Height;

            VENG_DEBUG("Window resize: %i %i", Width, Height);

            if(Width == 0 || Height == 0)
            {
                VENG_INFO("Window is minimized");
                AppState->IsSuspended = true;
                return true;
            }
            else
            {
                if(AppState->IsSuspended)
                {
                    VENG_INFO("Window is restored, resuming application");
                    AppState->IsSuspended = false;
                }

                AppState->GameInst->OnResize(AppState->GameInst, Width, Height);
                RendererOnResize(Width, Height);
            }
        }
    }

    return false;
}

