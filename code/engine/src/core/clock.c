#include "clock.h"
#include "platform/platform.h"


void ClockUpdate(clock* Clock)
{
    if(Clock->StartTime != 0)
    {
        Clock->Elapsed = PlatformGetAbsoluteTime() - Clock->StartTime;
    }
}

void ClockStart(clock* Clock)
{
    Clock->StartTime = PlatformGetAbsoluteTime();
    Clock->Elapsed = 0;
}

void ClockStop(clock* Clock)
{
    Clock->StartTime = 0;
}

