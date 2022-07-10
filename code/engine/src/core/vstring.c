#include "vstring.h"
#include "vmemory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(_MSC_VER)
#endif

u64 StringLength(const char* Str)
{
    return strlen(Str); 
}

char* StringDuplicate(const char* Str)
{
    u64 Length = StringLength(Str);
    char* Copy = Allocate(Length + 1, MEMORY_TAG_STRING);
    CopyMemory(Copy, Str, Length + 1);
    return Copy;
}

b8 IsStringsEqual(const char* A, const char* B)
{
    return strcmp(A, B) == 0;
}

b8 IsStringsEquali(const char* A, const char* B)
{
#if defined(__GNUC__)
    return strcasecmp(A, B) == 0;
#elif defined(_MSC_VER)
    return _stricmp(A, B) == 0;
#endif
}

VENG_API s32 StringFormat(char* Dest, const char* Format, ...)
{
    if(Dest)
    {
        __builtin_va_list ArgPtr;
        va_start(ArgPtr, Format);
        s32 Written = StringFormatV(Dest, Format, ArgPtr);
        va_end(ArgPtr);
        return Written;
    }
    return -1;
}

VENG_API s32 StringFormatV(char* Dest, const char* Format, void* va_listp)
{
    if(Dest)
    {
        char Buffer[32000];
        s32 Written = vsnprintf(Buffer, 32000, Format, va_listp);
        Buffer[Written] = 0;
        CopyMemory(Dest, Buffer, Written + 1);
        return Written;
    }
    return -1;
}

