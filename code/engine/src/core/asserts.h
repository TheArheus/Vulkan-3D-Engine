#pragma once

#include "defines.h"

#define VENG_ASSERTATIONS_ENABLED

#ifdef VENG_ASSERTATIONS_ENABLED

#if _MSC_VER
#include <intrin.h>
#define DebugBreak() __debugbreak()
#else
// #define DebugBreak() __builtin_trap()
#define DebugBreak() {*(int*)0 = 0;} 
#endif

VENG_API void ReportAssertationFailure(const char* Expression, const char* Message, const char* File, s32 Line);

#define Assert(Expr)                                                                \
    {                                                                               \
        if(Expr)                                                                    \
        {}                                                                          \
        else                                                                        \
        {                                                                           \
            ReportAssertationFailure(#Expr, "", __FILE__, __LINE__);           \
            DebugBreak();                                                           \
        }                                                                           \
    }


#define AssertMsg(Expr, Message)                                                    \
    {                                                                               \
        if(Expr)                                                                    \
        {}                                                                          \
        else                                                                        \
        {                                                                           \
            ReportAssertationFailure(#Expr, Message, __FILE__, __LINE__);      \
            DebugBreak();                                                           \
        }                                                                           \
    }

#ifdef _DEBUG
#define AssertDebug(Expr)                                                           \
    {                                                                               \
        if(Expr)                                                                    \
        {}                                                                          \
        else                                                                        \
        {                                                                           \
            ReportAssertationFailure(#Expr, "", __FILE__, __LINE__);           \
            DebugBreak();                                                           \
        }                                                                           \
    }
#else
#define AssertDebug(Expr)
#endif

#else
#define Assert(Expr)
#define AssertMsg(Expr)
#define AssertDebug(Expr)
#endif

