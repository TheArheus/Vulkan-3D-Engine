#include "vstring.h"
#include "vmemory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#if defined(_MSC_VER)
#endif

char* StringEmpty(char* Str)
{
    if(Str)
    {
        Str[0] = 0;
    }

    return Str;
}

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

VENG_API char* StringCopy(char* Dest, const char* Source)
{
    return strcpy(Dest, Source);
}

VENG_API char* StringCopyN(char* Dest, const char* Source, s64 Length)
{
    return strncpy(Dest, Source, Length);
}

VENG_API char* StringTrim(char* Str)
{
    while(isspace((unsigned char)*Str))
    {
        Str++;
    }
    if(*Str)
    {
        char* p = Str;
        while(*p)
        {
            p++;
        }
        while(isspace((unsigned char)*(--p)));
        p[1] = '\0';
    }

    return Str;
}

VENG_API void StringMid(char* Dest, const char* Source, s32 Start, s32 Length)
{
    if(Length == 0) return;

    u64 SrcLength = StringLength(Source);
    if(Start >= SrcLength)
    {
        Dest[0] = 0;
        return;
    }

    if(Length > 0)
    {
        for(u64 i = Start, j = 0; j < Length && Source[i]; ++i, ++j)
        {
            Dest[i] = Source[j];
        }

        Dest[Start + Length] = 0;
    }
    else
    {
        u64 j = 0;
        for(u64 i = Start; Source[i]; ++i, ++j)
        {
            Dest[j] = Source[i];
        }

        Dest[Start + j] = 0;
    }
}

VENG_API s32 StringIndexOf(char* Str, char c)
{
    if(!Str)
    {
        return -1;
    }

    u64 Length = StringLength(Str);
    if(Length > 0)
    {
        for(u32 i = 0; i < Length; ++i)
        {
            if(Str[i] == c) return i;
        }
    }

    return -1;
}

VENG_API b8 StringToVec4(char* Str, v4* OutVector)
{
    if(!Str) return false;

    ZeroMemory(OutVector, sizeof(v4));
    s32 Result = sscanf(Str, "%f %f %f %f", &OutVector->x, &OutVector->y, &OutVector->z, &OutVector->w);
    return Result != -1;
}

VENG_API b8 StringToVec3(char* Str, v3* OutVector)
{
    if(!Str) return false;

    ZeroMemory(OutVector, sizeof(v3));
    s32 Result = sscanf(Str, "%f %f %f", &OutVector->x, &OutVector->y, &OutVector->z);
    return Result != -1;
}

VENG_API b8 StringToVec2(char* Str, v2* OutVector)
{
    if(!Str) return false;

    ZeroMemory(OutVector, sizeof(v2));
    s32 Result = sscanf(Str, "%f %f", &OutVector->x, &OutVector->y);
    return Result != -1;
}

VENG_API b8 StringToR64(char* Str, r64* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%lf", V);
    return Result != -1;
}

VENG_API b8 StringToR32(char* Str, r32* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%f", V);
    return Result != -1;
}

VENG_API b8 StringToU64(char* Str, u64* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%llu", V);
    return Result != -1;
}

VENG_API b8 StringToU32(char* Str, u32* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%u", V);
    return Result != -1;
}

VENG_API b8 StringToU16(char* Str, u16* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%hu", V);
    return Result != -1;
}

VENG_API b8 StringToU8(char* Str, u8* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%hhu", V);
    return Result != -1;
}

VENG_API b8 StringToS64(char* Str, s64* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%lli", V);
    return Result != -1;
}

VENG_API b8 StringToS32(char* Str, s32* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%i", V);
    return Result != -1;
}

VENG_API b8 StringToS16(char* Str, s16* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%hi", V);
    return Result != -1;
}

VENG_API b8 StringToS8(char* Str, s8* V)
{
    if(!Str) return false;

    *V = 0;
    s32 Result = sscanf(Str, "%hhi", V);
    return Result != -1;
}

VENG_API b8 StringToBool(char* Str, b8* V)
{
    if(!Str) return false;

    return IsStringsEqual(Str, "1") || IsStringsEqual(Str, "true");
}

