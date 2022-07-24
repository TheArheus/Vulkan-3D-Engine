#pragma once

#include "defines.h"
#include "math/math_types.h"

VENG_API char* StringEmpty(char* Str);

VENG_API u64 StringLength(const char* Str);
VENG_API char* StringDuplicate(const char* Str);

VENG_API b8 IsStringsEqual(const char* A, const char* B);
b8 IsStringsEquali(const char* A, const char* B);

VENG_API s32 StringFormat(char* Dest, const char* Format, ...);
VENG_API s32 StringFormatV(char* Dest, const char* Format, void* va_listp);

VENG_API char* StringCopy(char* Dest, const char* Source);
VENG_API char* StringCopyN(char* Dest, const char* Source, s64 Length);

VENG_API char* StringTrim(char* Str);

VENG_API void StringMid(char* Dest, const char* Source, s32 Start, s32 Length);

VENG_API s32 StringIndexOf(char* Str, char c);


VENG_API b8 StringToVec4(char* Str, v4* OutVector);
VENG_API b8 StringToVec3(char* Str, v3* OutVector);
VENG_API b8 StringToVec2(char* Str, v2* OutVector);

VENG_API b8 StringToR64(char* Str, r64* V);
VENG_API b8 StringToR32(char* Str, r32* V);

VENG_API b8 StringToU64(char* Str, u64* V);
VENG_API b8 StringToU32(char* Str, u32* V);
VENG_API b8 StringToU16(char* Str, u16* V);
VENG_API b8 StringToU8(char* Str, u8* V);

VENG_API b8 StringToS64(char* Str, s64* V);
VENG_API b8 StringToS32(char* Str, s32* V);
VENG_API b8 StringToS16(char* Str, s16* V);
VENG_API b8 StringToS8(char* Str, s8* V);

VENG_API b8 StringToBool(char* Str, b8* V);
