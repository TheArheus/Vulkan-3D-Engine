#pragma once

#include "defines.h"


b8 PlatformStartup(u64* MemoryRequirement, void* State, 
                   const char* ApplicationName, 
                   s32 X, s32 Y, s32 Width, s32 Height);

void PlatformShutdown(void* State);

b8 PlatformPumpMessages();

void* PlatformAllocate(u64 Size, b8 Aligned);
void PlatformFree(void* Block, b8 Aligned);
void* PlatformZeroMemory(void* Block, u64 Size);
void* PlatformCopyMemory(void* Dest, const void* Source, u64 Size);
void* PlatformSetMemory(void* Dest, s32 Values, u32 Size);

void PlatformConsoleWrite(const char* Message, u8 Color);
void PlatformConsoleWriteError(const char* Message, u8 Color);

r64 PlatformGetAbsoluteTime();

void PlatformSleep(u64 Millis);

