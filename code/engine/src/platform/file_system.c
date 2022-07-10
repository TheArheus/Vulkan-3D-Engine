#include "file_system.h"

#include "core/logger.h"
#include "core/vmemory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


b8 FileExists(const char* Path)
{
    struct stat buff;
    return stat(Path, &buff) == 0;
}

b8 FileOpen(const char* Path, file_modes Mode, b8 Binary, file_handle* OutHandle)
{
    OutHandle->IsValid = true;
    OutHandle->Handle = 0;

    const char* ModeStr;

    if((Mode & FILE_MODE_READ) != 0 && (Mode & FILE_MODE_WRITE) != 0)
    {
        ModeStr = Binary ? "w+b" : "w+";
    }
    else if((Mode & FILE_MODE_READ) != 0 && (Mode & FILE_MODE_WRITE) == 0)
    {
        ModeStr = Binary ? "rb" : "r";
    }
    else if((Mode & FILE_MODE_READ) == 0 && (Mode & FILE_MODE_WRITE) != 0)
    {
        ModeStr = Binary ? "wb" : "w";
    }
    else
    {
        VENG_ERROR("Invalid mode passed while trying to open file: %s", Path);
        return false;
    }

    FILE* File = fopen(Path, ModeStr);
    if(!File)
    {
        VENG_ERROR("Error opening file: %s", Path);
        return false;
    }

    OutHandle->Handle  = File;
    OutHandle->IsValid = true;

    return true;
}

void FileClose(file_handle* Handle)
{
    if(Handle)
    {
        fclose((FILE*)Handle->Handle);
        Handle->Handle  = 0;
        Handle->IsValid = false;
    }
}

b8 FileReadLine(file_handle* Handle, char** Line)
{
    if(Handle->Handle)
    {
        char Buffer[32000];
        if(fgets(Buffer, 32000, (FILE*)Handle->Handle) != 0)
        {
            u64 Length = strlen(Buffer);
            *Line = Allocate(sizeof(char) * Length + 1, MEMORY_TAG_STRING);
            strcpy(*Line, Buffer);
            return true;
        }
    }

    return false;
}

b8 FileWriteLine(file_handle* Handle, const char* Line)
{
    if(Handle->Handle)
    {
        s32 Result = fputs(Line, (FILE*)Handle->Handle);
        if(Result != EOF)
        {
            Result = fputc('\n', (FILE*)Handle->Handle);
        }

        fflush((FILE*)Handle->Handle);
        return Result != EOF;
    }

    return false;
}

b8 FileRead(file_handle* Handle, u64 Size, void* OutData, u64* OutBytesRead)
{
    if(Handle->Handle && OutData)
    {
        *OutBytesRead = fread(OutData, 1, Size, (FILE*)Handle->Handle);
        if(*OutBytesRead != Size)
        {
            return false;
        }
        return true;
    }
    return false;
}

b8 FileReadAllBytes(file_handle* Handle, u8** OutData, u64* OutBytesRead)
{
    if(Handle->Handle)
    {
        fseek((FILE*)Handle->Handle, 0, SEEK_END);
        u64 Size = ftell((FILE*)Handle->Handle);
        rewind((FILE*)Handle->Handle);

        *OutData = Allocate(sizeof(char) * Size, MEMORY_TAG_STRING);
        *OutBytesRead = fread(*OutData, 1, Size, (FILE*)Handle->Handle);
        if(*OutBytesRead != Size)
        {
            return false;
        }
        return true;
    }
    return true;
}

b8 FileWrite(file_handle* Handle, u64 Size, const void* Data, u64* OutBytesWrite)
{
    if(Handle->Handle)
    {
        *OutBytesWrite = fwrite(Data, 1, Size, (FILE*)Handle->Handle);
        if(*OutBytesWrite != Size)
        {
            return false;
        }
        fflush((FILE*)Handle->Handle);
        return true;
    }
    return false;
}

