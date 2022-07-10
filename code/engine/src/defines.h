#pragma once

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef char                s8;
typedef short               s16;
typedef int                 s32;
typedef long long           s64;

typedef float               r32;
typedef double              r64;

typedef s32                 b32;
typedef _Bool               b8;

#define true  1
#define false 0

#define INVALID_ID -1

#if 1
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define VENG_WINDOWS 1
#ifndef _WIN64
#error "64-bit required for windows"
#endif
#elif defined(__linux__) || define(__gnu_linux__)
#elif defined(__unix__)
#elif defined(_POSIX_VERSION)
#elif __APPLE__
#else
#error "Unknown plarform"
#endif
#endif

#if VENG_EXPORT
#ifdef _MSC_VER
#define VENG_API __declspec(dllexport)
#else
#define VENG_API __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define VENG_API __declspec(dllimport)
#else
#define VENG_API
#endif
#endif

#define Clamp(Min, X, Max) ((X < Min) ? Min : (X > Max ? Max : X))
#define alignas _Alignas

#ifdef _MSC_VER
#define INLINE   __forceinline
#define NOINLINE __declspec(noinline)
#else
#define INLINE   static inline 
#define NOINLINE 
#endif

