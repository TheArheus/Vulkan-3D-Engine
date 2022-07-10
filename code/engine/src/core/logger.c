
#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"
#include "platform/file_system.h"

#include "vstring.h"
#include "vmemory.h"

#include <stdarg.h>

typedef struct logger__system_state
{
    file_handle LogFile;
} logger_system_state;

static logger_system_state* LoggerState;

void AppendToLogFile(const char* Message)
{
    if(LoggerState && LoggerState->LogFile.IsValid)
    {
        u64 Length = StringLength(Message);
        u64 Written = 0;
        if(!FileWrite(&LoggerState->LogFile, Length, Message, &Written))
        {
            PlatformConsoleWriteError("ERROR. Unable to write to console log.", LOG_LEVEL_ERROR);
        }
    }
}

b8 InitializeLogging(u64* MemoryRequirement, void* State)
{
    *MemoryRequirement = sizeof(logger_system_state);
    if(State == 0)
    {
        return true;
    }

    LoggerState = State;
    if(!FileOpen("console.log", FILE_MODE_WRITE, false, &LoggerState->LogFile))
    {
        PlatformConsoleWriteError("ERROR. Unable to openg console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    return true;
}

void ShutdownLogging(void* State)
{
    State = 0;
}

void OutputLog(log_level Level, const char* Message, ...)
{
    const char* LevelString[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 IsError = Level < LOG_LEVEL_WARN;

    char OutMessage[32000];
    ZeroMemory(OutMessage, sizeof(OutMessage));

    va_list ArgumentPointer;
    va_start(ArgumentPointer, Message);
    StringFormatV(OutMessage, Message, ArgumentPointer);
    va_end(ArgumentPointer);

    StringFormat(OutMessage, "%s%s\n", LevelString[Level], OutMessage);

    if(IsError)
    {
        PlatformConsoleWrite(OutMessage, Level);
    }
    else
    {
        PlatformConsoleWriteError(OutMessage, Level);
    }

    AppendToLogFile(OutMessage);
}

void ReportAssertationFailure(const char* Expression, const char* Message, const char* File, s32 Line)
{
    OutputLog(LOG_LEVEL_FATAL, "Assertation Failure: %s, message: %s in file %s in line: %d", Expression, Message, File, Line);
}


