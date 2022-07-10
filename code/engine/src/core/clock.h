#pragma once

#include "defines.h"

typedef struct clock
{
    r64 StartTime;
    r64 Elapsed;
} clock;

void ClockUpdate(clock* Clock);
void ClockStart(clock* Clock);
void ClockStop(clock* Clock);
