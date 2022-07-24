#include "file_system.h"

#include "core/logger.h"
#include "core/vmemory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


b8 FileExists(const char* Path)
{
#if _MSC_VER
    struct _stat buff;
    return _stat(Path, &buff);
#else
    struct stat buff;
    return stat(Path, &buff) == 0;
#endif
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

b8 FileSize(file_handle* Handle, u64* OutSize)
{
    if(Handle->Handle)
    {
        fseek((FILE*)Handle->Handle, 0, SEEK_END);
        *OutSize = ftell((FILE*)Handle->Handle);
        rewind((FILE*)Handle->Handle);
        return true;
    }
    return false;
}

b8 FileReadLine(file_handle* FileHandle, u64 MaxLength, char** LineBuf, u64* OutLineLength)
{
    if(FileHandle->Handle && LineBuf && OutLineLength && MaxLength > 0)
    {
        char* Buffer = *LineBuf;
        if(fgets(Buffer, MaxLength, (FILE*)FileHandle->Handle) != 0)
        {
            *OutLineLength = strlen(*LineBuf);
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

b8 FileReadAllBytes(file_handle* Handle, u8* OutData, u64* OutBytesRead)
{
    if(Handle->Handle)
    {
        u64 Size = 0;
        if(!FileSize(Handle, &Size))
        {
            return false;
        }

        *OutBytesRead = fread(OutData, 1, Size, (FILE*)Handle->Handle);

        return *OutBytesRead == Size;
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

b8 FileReadAllText(file_handle* Handle, char* OutText, u64* OutBytesRead)
{
    if(Handle->Handle && OutText && OutBytesRead)
    {
        u64 Size = 0;
        if(!FileSize(Handle, &Size))
        {
            return false;
        }

        *OutBytesRead = fread(OutText, 1, Size, (FILE*)Handle->Handle);
        return *OutBytesRead == Size;
    }
}

