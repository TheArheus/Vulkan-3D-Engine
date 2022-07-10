#pragma once

#include "defines.h"

VENG_API u64 StringLength(const char* Str);
VENG_API char* StringDuplicate(const char* Str);
VENG_API b8 IsStringsEqual(const char* A, const char* B);
b8 IsStringsEquali(const char* A, const char* B);
VENG_API s32 StringFormat(char* Dest, const char* Format, ...);
VENG_API s32 StringFormatV(char* Dest, const char* Format, void* va_listp);
