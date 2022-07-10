#include "event.h"
#include "core/vmemory.h"
#include "containers/darray.h"

typedef struct registered_event
{
    void* Listener;
    PFN_OnEvent Callback;
} registered_event;

typedef struct event_code_entry
{
    registered_event* Events;
} event_code_entry;

#define MAX_MESSAGE_CODES 16384

typedef struct event_system_state
{
    event_code_entry Registered[MAX_MESSAGE_CODES];
} event_system_state;

static event_system_state* EventState;


void EventInitialize(u64* MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(event_system_state);
    if(State == 0)
    {
        return;
    }

    ZeroMemory(State, sizeof(event_system_state));
    EventState = State;
}

void EventShutdown(void* State)
{
    if(EventState)
    {
        for(s16 EventIndex = 0;
            EventIndex < MAX_MESSAGE_CODES;
            ++EventIndex)
        {
            if(EventState->Registered[EventIndex].Events != 0)
            {
                DArrayDestroy(EventState->Registered[EventIndex].Events);
                EventState->Registered[EventIndex].Events = 0;
            }
        }
    }

    EventState = 0;
}

b8 EventRegister(u16 Code, void* Listener, PFN_OnEvent OnEvent)
{
    if(!EventState)
    {
        return false;
    }

    if(EventState->Registered[Code].Events == 0)
    {
        EventState->Registered[Code].Events = (registered_event*)DArrayCreate(registered_event);
    }

    u64 RegisteredCount = DArrayLength(EventState->Registered[Code].Events);
    for(u32 EventIndex = 0;
        EventIndex < RegisteredCount;
        ++EventIndex)
    {
        if(EventState->Registered[Code].Events[EventIndex].Listener == Listener)
        {
            return false;
        }
    }

    registered_event Event;
    Event.Listener = Listener;
    Event.Callback = OnEvent;
    DArrayPush(EventState->Registered[Code].Events, Event);

    return true;
}

b8 EventUnregister(u16 Code, void* Listener, PFN_OnEvent OnEvent)
{
    if(!EventState)
    {
        return false;
    }

    if(EventState->Registered[Code].Events == 0)
    {
        return false;
    }

    u64 RegisteredCount = DArrayLength(EventState->Registered[Code].Events);
    for(u32 EventIndex = 0;
        EventIndex < RegisteredCount;
        ++EventIndex)
    {
        registered_event Event = EventState->Registered[Code].Events[EventIndex];
        if(Event.Listener == Listener && Event.Callback == OnEvent)
        {
            registered_event PopedEvent;
            DArrayPopAt(EventState->Registered[Code].Events, EventIndex, &PopedEvent);
            return true;
        }
    }

    return false;
}
b8 EventFire(u16 Code, void* Sender, event_context Context)
{
    if(!EventState)
    {
        return false;
    }

    if(EventState->Registered[Code].Events == 0)
    {
        return false;
    }

    u64 RegisteredCount = DArrayLength(EventState->Registered[Code].Events);
    for(u32 EventIndex = 0;
        EventIndex < RegisteredCount;
        ++EventIndex)
    {
        registered_event Event = EventState->Registered[Code].Events[EventIndex];
        if(Event.Callback(Code, Sender, Event.Listener, Context))
        {
            return true;
        }
    }

    return false;
}

