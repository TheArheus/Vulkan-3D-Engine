#pragma once

#include "core/application.h"

typedef struct game
{
    application_config AppConfig;
    b8   (*Initialize)(struct game* GameInst);
    b8   (*Update)(struct game* GameInst, r32 DeltaTime);
    b8   (*Render)(struct game* GameInst, r32 DeltaTime);
    void (*OnResize)(struct game* GameInst, u32 Width, u32 Height);

    void* State;
    void* ApplicationState;
} game;

