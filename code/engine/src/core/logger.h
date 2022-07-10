#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED  1
#define LOG_INFO_ENABLED  1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if VENG_RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN  = 2,
    LOG_LEVEL_INFO  = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,
} log_level;

b8 InitializeLogging(u64* MemoryRequirement, void* State);
void ShutdownLogging(void* State);

VENG_API void OutputLog(log_level Level, const char* Message, ...);

#define VENG_FATAL(Message, ...) OutputLog(LOG_LEVEL_FATAL, Message, ##__VA_ARGS__);

#ifndef VENG_ERROR
#define VENG_ERROR(Message, ...) OutputLog(LOG_LEVEL_ERROR, Message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define VENG_WARN(Message, ...) OutputLog(LOG_LEVEL_WARN, Message, ##__VA_ARGS__);
#else
#define VENG_WARN(Message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define VENG_INFO(Message, ...) OutputLog(LOG_LEVEL_INFO, Message, ##__VA_ARGS__);
#else
#define VENG_INFO(Message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define VENG_DEBUG(Message, ...) OutputLog(LOG_LEVEL_DEBUG, Message, ##__VA_ARGS__);
#else
#define VENG_DEBUG(Message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define VENG_TRACE(Message, ...) OutputLog(LOG_LEVEL_TRACE, Message, ##__VA_ARGS__);
#else
#define VENG_TRACE(Message, ...)
#endif

