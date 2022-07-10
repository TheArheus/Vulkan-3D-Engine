#pragma once

#include "defines.h"

struct game;

typedef struct application_config
{
    s16 StartX;
    s16 StartY;

    s16 StartWidth;
    s16 StartHeight;

    char *Name;
} application_config;


VENG_API b8 ApplicationCreate(struct game* GameInst);
VENG_API b8 ApplicationRun();
void ApplicationGetFramebufferSize(u32* Width, u32* Height);

