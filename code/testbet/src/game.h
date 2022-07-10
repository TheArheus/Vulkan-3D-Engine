#pragma once

#include <defines.h>
#include <game_types.h>
#include <math/math_types.h>

typedef struct game_state
{
    r32 DeltaTime;
    mat4 View;
    v3 CameraPosition;
    v3 CameraRotation;
    b8 CameraViewDirty;
} game_state;


b8 Initialize(struct game* GameInst);
b8 Update(struct game* GameInst, r32 DeltaTime);
b8 Render(struct game* GameInst, r32 DeltaTime);
void OnResize(struct game* GameInst, u32 Width, u32 Height);

