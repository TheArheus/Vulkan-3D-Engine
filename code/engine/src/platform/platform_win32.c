#include "platform.h"

#if VENG_WINDOWS

#include "core/logger.h"
#include "core/input.h"
#include "core/event.h"
#include "containers/darray.h"

#include "renderer/vulkan/vulkan_platform.h"

#include <windows.h>
#include <windowsx.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.inl"

typedef struct platform_state
{
    HINSTANCE Instance;
    HWND WindowHandle;
    VkSurfaceKHR Surface;
} platform_state;

static platform_state* PlatformState;
static r64 ClockFrequency;
static LARGE_INTEGER StartTime;


LRESULT CALLBACK 
Win32ProcessMessage(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam);

b8 PlatformStartup(u64* MemoryRequirement, void* State, const char* ApplicationName, s32 X, s32 Y, s32 Width, s32 Height)
{
    *MemoryRequirement = sizeof(platform_state);
    if(State == 0)
    {
        return true;
    }

    PlatformState = State;
    PlatformState->Instance = GetModuleHandleA(0);

    const char ClassName[] = "VENG_WINDOW_CLASS";

    HICON Icon = LoadIcon(PlatformState->Instance, IDI_APPLICATION);

    WNDCLASSA Class;
    memset(&Class, 0, sizeof(Class));
    Class.style = CS_DBLCLKS;
    Class.lpfnWndProc = Win32ProcessMessage;
    Class.hIcon = Icon;
    Class.hInstance = PlatformState->Instance;
    Class.hCursor = LoadCursor(NULL, IDC_ARROW);
    Class.lpszClassName = ClassName;

    if(!RegisterClassA(&Class))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    u32 WindowX = X;
    u32 WindowY = Y;
    u32 WindowWidth  = Width;
    u32 WindowHeight = Height;

    u32 WindowStyle   = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 WindowExStyle = WS_EX_APPWINDOW;

    WindowStyle |= WS_MAXIMIZEBOX;
    WindowStyle |= WS_MINIMIZEBOX;
    WindowStyle |= WS_THICKFRAME;

    RECT BorderRect = {0, 0, 0, 0};
    AdjustWindowRectEx(&BorderRect, WindowStyle, 0, WindowExStyle);

    WindowX += BorderRect.left;
    WindowY += BorderRect.top;

    WindowWidth  += BorderRect.right - BorderRect.left;
    WindowHeight += BorderRect.top   - BorderRect.bottom;

    HWND Handle = CreateWindowExA(WindowExStyle, ClassName, ApplicationName,
                                 WindowStyle, WindowX, WindowY, WindowWidth, WindowHeight,
                                 0, 0, PlatformState->Instance, 0);

    if(Handle == 0)
    {
        MessageBoxA(0, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }
    else
    {
        PlatformState->WindowHandle = Handle;
    }

    b32 ShouldActivate = 1;
    s32 ShowWindowFlags = ShouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow(PlatformState->WindowHandle, ShowWindowFlags);

    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    ClockFrequency = 1.0f / Frequency.QuadPart;
    QueryPerformanceCounter(&StartTime);

    return true;
}

void PlatformShutdown(void* State)
{
    if(PlatformState && PlatformState->WindowHandle)
    {
        DestroyWindow(PlatformState->WindowHandle);
        PlatformState->WindowHandle = 0;
    }
}

b8 PlatformPumpMessages()
{
    if(PlatformState)
    {
        MSG Message;
        while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }
    }

    return true;
}

void* PlatformAllocate(u64 Size, b8 Aligned)
{
    return malloc(Size);
}
void PlatformFree(void* Block, b8 Aligned)
{
    free(Block);
}
void* PlatformZeroMemory(void* Block, u64 Size)
{
    return memset(Block, 0, Size);
}
void* PlatformCopyMemory(void* Dest, const void* Source, u64 Size)
{
    return memcpy(Dest, Source, Size);
}
void* PlatformSetMemory(void* Dest, s32 Values, u32 Size)
{
    return memset(Dest, Values, Size);
}

void PlatformConsoleWrite(const char* Message, u8 Color)
{
    static u8 Levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Levels[Color]);

    OutputDebugStringA(Message);

    u64 Length = strlen(Message);
    LPDWORD NumberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), Message, (DWORD)Length, NumberWritten, 0);
}
void PlatformConsoleWriteError(const char* Message, u8 Color)
{
    static u8 Levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), Levels[Color]);

    OutputDebugStringA(Message);

    u64 Length = strlen(Message);
    LPDWORD NumberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), Message, (DWORD)Length, NumberWritten, 0);
}

r64 PlatformGetAbsoluteTime()
{
    LARGE_INTEGER Now;
    QueryPerformanceCounter(&Now);
    return (r64)Now.QuadPart * ClockFrequency;
}

void PlatformSleep(u64 Millis)
{
    Sleep(Millis);
}

void PlatformGetRequiredExtensionNames(const char*** ExtensionNames)
{
    DArrayPush(*ExtensionNames, &"VK_KHR_win32_surface");
}

b8 PlatformCreateVulkanSurface(vulkan_context* Context)
{
    if(!PlatformState)
    {
        return false;
    }

    VkWin32SurfaceCreateInfoKHR CreateInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    CreateInfo.hinstance = PlatformState->Instance;
    CreateInfo.hwnd      = PlatformState->WindowHandle;

    VkResult Result = vkCreateWin32SurfaceKHR(Context->Instance, &CreateInfo, Context->Allocator, &PlatformState->Surface);
    if(Result != VK_SUCCESS)
    {
        VENG_ERROR("Could not create surface. Exiting");
        return false;
    }

    Context->Surface = PlatformState->Surface;
    return true;
}

LRESULT CALLBACK 
Win32ProcessMessage(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_ERASEBKGND:
        {
            return 1;
        } break;

        case WM_CLOSE:
        {
            event_context Data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, Data);
            return true;
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        } break;

        case WM_SIZE:
        {
            RECT R;
            GetClientRect(Window, &R);

            u32 Width = R.right - R.left;
            u32 Height = R.bottom - R.top;

            event_context Context;
            Context.data.Unsigned16[0] = (u16)Width;
            Context.data.Unsigned16[1] = (u16)Height;
            EventFire(EVENT_CODE_RESIZED, 0, Context);
        } break;
        
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
#if 0
            b8 IsPressed  = ((lParam & (1 << 31)) == 0);
            b8 WasPressed = ((lParam & (1 << 30)) != 0);
#else
            b8 IsPressed = (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN);
#endif
            keys Key = (u16)wParam;

            b8 IsExtended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

            if(wParam == VK_MENU)
            {
                Key = IsExtended ? KEY_RALT : KEY_LALT;
            }
            else if(wParam == VK_SHIFT)
            {
                u32 LeftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                u32 Scancode = ((lParam & (0xFF << 16)) >> 16);
                Key = Scancode == LeftShift ? KEY_LSHIFT : KEY_RSHIFT;
            }
            else if(wParam == VK_CONTROL)
            {
                Key = IsExtended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            ProcessKey(Key, IsPressed);

            return 0;
        } break;

        case WM_MOUSEMOVE:
        {
            s32 PosX = GET_X_LPARAM(lParam);
            s32 PosY = GET_Y_LPARAM(lParam);

            ProcessMouseMove(PosX, PosY);
        } break;

        case WM_MOUSEWHEEL:
        {
            s32 DeltaZ = GET_WHEEL_DELTA_WPARAM(wParam);
            if(DeltaZ != 0)
            {
                DeltaZ = (DeltaZ < 0) ? -1 : 1;
                ProcessMouseWheel(DeltaZ);
            }
        } break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            b8 IsPressed = Message == WM_LBUTTONDOWN || Message == WM_RBUTTONDOWN || Message == WM_MBUTTONDOWN;
            buttons Button = BUTTON_MAX_BUTTONS;
            switch(Message)
            {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    Button = BUTTON_LEFT;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    Button = BUTTON_RIGHT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    Button = BUTTON_MIDDLE;
                    break;
            }

            if(Button != BUTTON_MAX_BUTTONS)
                ProcessButton(Button, IsPressed);
        } break;
    }

    return DefWindowProcA(Window, Message, wParam, lParam);
}

#endif
