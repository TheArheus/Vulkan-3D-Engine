#pragma once

#include "defines.h"

typedef struct file_handle
{
    void* Handle;
    b8 IsValid;
} file_handle;

typedef enum file_modes
{
    FILE_MODE_READ  = 0x1,
    FILE_MODE_WRITE = 0x2,
} file_modes;

VENG_API b8 FileExists(const char* Path);
VENG_API b8 FileOpen(const char* Path, file_modes Mode, b8 Binary, file_handle* Handle);
VENG_API void FileClose(file_handle* Handle);

VENG_API b8 FileReadLine(file_handle* Handle, char** Line);
VENG_API b8 FileWriteLine(file_handle* Handle, const char* Line);
VENG_API b8 FileRead(file_handle* Handle, u64 Size, void* OutData, u64* OutBytesRead);
VENG_API b8 FileReadAllBytes(file_handle* Handle, u8** OutData, u64* OutBytesRead);
VENG_API b8 FileWrite(file_handle* Handle, u64 Size, const void* Data, u64* OutBytesWrite);
