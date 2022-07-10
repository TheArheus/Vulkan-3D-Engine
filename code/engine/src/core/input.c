#include "input.h"
#include "core/event.h"
#include "core/vmemory.h"
#include "core/logger.h"


typedef struct keyboard_state
{
    b8 Keys[256];
} keyboard_state;

typedef struct mouse_state
{
    s16 X;
    s16 Y;
    u8 Buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state
{
    keyboard_state KeyboardCurr;
    keyboard_state KeyboardPrev;
    mouse_state MouseCurr;
    mouse_state MousePrev;
} input_state;

static input_state* InputState;

void InitializeInput(u64* MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(input_state);
    if(State == 0)
    {
        return;
    }

    ZeroMemory(State, sizeof(input_state));
    InputState = State;
}

void InputShutdown(void* State)
{
    InputState = 0;
}

void InputUpdate(r64 DeltaTime)
{
    if(!InputState)
    {
        return;
    }

    CopyMemory(&InputState->KeyboardPrev, &InputState->KeyboardCurr, sizeof(keyboard_state));
    CopyMemory(&InputState->MousePrev, &InputState->MouseCurr, sizeof(mouse_state));
}

b8 IsKeyDown(keys Key)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->KeyboardCurr.Keys[Key] == true;
}
b8 IsKeyUp(keys Key)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->KeyboardCurr.Keys[Key] == false;
}
b8 WasKeyDown(keys Key)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->KeyboardPrev.Keys[Key] == true;
}
b8 WasKeyUp(keys Key)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->KeyboardPrev.Keys[Key] == false;
}

void ProcessKey(keys Key, b8 Pressed)
{
    if(InputState && InputState->KeyboardCurr.Keys[Key] != Pressed)
    {
        InputState->KeyboardCurr.Keys[Key] = Pressed;

        event_context Context;
        Context.data.Unsigned16[0] = Key;
        EventFire(Pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, Context);
    }
}

b8 IsButtonDown(buttons Button)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->MouseCurr.Buttons[Button] == true;
}
b8 IsButtonUp(buttons Button)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->MouseCurr.Buttons[Button] == false;
}
b8 WasButtonDown(buttons Button)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->MousePrev.Buttons[Button] == true;
}
b8 WasButtonUp(buttons Button)
{
    if(!InputState)
    {
        return false;
    }

    return InputState->MousePrev.Buttons[Button] == false;
}
void GetMousePos(s32* X, s32* Y)
{
    if(!InputState)
    {
        *X = 0;
        *Y = 0;
        return;
    }

    *X = InputState->MouseCurr.X;
    *Y = InputState->MouseCurr.Y;
}
void GetPrevMousePos(s32* X, s32* Y)
{
    if(!InputState)
    {
        *X = 0;
        *Y = 0;
        return;
    }

    *X = InputState->MousePrev.X;
    *Y = InputState->MousePrev.Y;
}

void ProcessButton(buttons Button, b8 Pressed)
{
    if(InputState->MouseCurr.Buttons[Button] != Pressed)
    {
        InputState->MouseCurr.Buttons[Button] = Pressed;

        event_context Context;
        Context.data.Unsigned16[0] = Button;
        EventFire(Pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, Context);
    }
}
void ProcessMouseMove(s16 X, s16 Y)
{
    if(InputState->MouseCurr.X != X && InputState->MouseCurr.Y != Y)
    {
        InputState->MouseCurr.X = X;
        InputState->MouseCurr.Y = Y;

        event_context Context;
        Context.data.Unsigned16[0] = X;
        Context.data.Unsigned16[1] = Y;
        EventFire(EVENT_CODE_MOUSE_MOVED, 0, Context);
    }
}
void ProcessMouseWheel(s8 DeltaZ)
{
    event_context Context;
    Context.data.Unsigned8[0] = DeltaZ;
    EventFire(EVENT_CODE_MOUSE_WHEEL, 0, Context);
}


