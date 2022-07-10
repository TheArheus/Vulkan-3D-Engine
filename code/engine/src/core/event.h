#pragma once

#include "defines.h"

typedef struct event_context
{
    union
    {
        s64 Signed64[2];
        u64 Unsigned64[2];
        r64 Real64[2];

        s32 Signed32[4];
        u32 Unsigned32[4];
        r32 Real32[4];

        s16 Signed16[8];
        u16 Unsigned16[8];

        s8 Signed8[16];
        u8 Unsigned8[16];

        char c[16];
    } data;
} event_context;

typedef b8 (*PFN_OnEvent)(u16 Code, void* Sender, void* ListenerInst, event_context Data);

void EventInitialize(u64* MemoryRequirement, void* State);
void EventShutdown(void* State);

VENG_API b8 EventRegister(u16 Code, void* Listener, PFN_OnEvent OnEvent);
VENG_API b8 EventUnregister(u16 Code, void* Listener, PFN_OnEvent OnEvent);
VENG_API b8 EventFire(u16 Code, void* Sender, event_context Context);

typedef enum system_event_code
{
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    EVENT_CODE_KEY_PRESSED = 0x02,
    EVENT_CODE_KEY_RELEASED = 0x03,

    EVENT_CODE_BUTTON_PRESSED = 0x04,
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    EVENT_CODE_MOUSE_MOVED = 0x06,
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    EVENT_CODE_RESIZED = 0x08,

    EVENT_CODE_DEBUG0 = 0X10,
    EVENT_CODE_DEBUG1 = 0X11,
    EVENT_CODE_DEBUG2 = 0X12,
    EVENT_CODE_DEBUG3 = 0X13,
    EVENT_CODE_DEBUG4 = 0X14,

    MAX_EVENT_CODE = 0xFF
} system_event_code;

