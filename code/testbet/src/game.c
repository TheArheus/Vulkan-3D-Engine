
#include "game.h"

#include <core/logger.h>
#include <core/input.h>
#include <core/event.h>
#include <math/vmath.h>

#include <renderer/renderer_frontend.h>

void RecalculateView(game_state* State)
{
    if(State->CameraViewDirty)
    {
        mat4 TranslationRes = Translation(State->CameraPosition);
        mat4 RotationRes = EulerXYZ(State->CameraRotation);

        State->View = MulMat4(RotationRes, TranslationRes);
        State->View = Inverse(State->View);

        State->CameraViewDirty = false;
    }
}

void CameraYaw(game_state* State, r32 Value)
{
    State->CameraRotation.y += Value;
    State->CameraViewDirty = true;
}

void CameraPitch(game_state* State, r32 Value)
{
    State->CameraRotation.x += Value;
    
    r32 Limit = DegToRad(89.0f);
    State->CameraPosition.x = Clamp(-Limit, State->CameraPosition.x, Limit);

    State->CameraViewDirty = true;
}

// NOTE: main functions of game to work with
b8 Initialize(struct game* GameInst)
{
    game_state* GameState = ((game_state*)GameInst->State);

    GameState->CameraPosition = V3(0, 0, 50.0f);
    GameState->CameraRotation = V3Zero();

    GameState->View = Translation(GameState->CameraPosition);
    GameState->View = Inverse(GameState->View);
    GameState->CameraViewDirty = true;

    return true;
}

b8 Update(struct game* GameInst, r32 DeltaTime)
{
    game_state* State = (game_state*)GameInst->State;

    if(IsKeyUp('T') && WasKeyDown('T'))
    {
        VENG_DEBUG("Texture swaping!");
        event_context Context = {};
        EventFire(EVENT_CODE_DEBUG0, GameInst, Context);
    }

    r32 TempMoveSpeed = 50.0f;
    v3 Velocity = V3Zero();

    static r32 Z = 0.0f;
    Z -= 0.0001f;
    AddV3(State->CameraPosition, V3(0, 0, Z));

    if(IsKeyDown('A') || IsKeyDown(KEY_LEFT))
    {
        CameraPitch(State, -1.0f * DeltaTime);
    }
    if(IsKeyDown('D') || IsKeyDown(KEY_RIGHT))
    {
        CameraPitch(State,  1.0f * DeltaTime);
    }
    if(IsKeyDown(KEY_UP))
    {
        CameraYaw(State,  1.0f * DeltaTime);
    }
    if(IsKeyDown(KEY_DOWN))
    {
        CameraYaw(State, -1.0f * DeltaTime);
    }

    if(IsKeyDown('W'))
    {
        v3 ForwardSpeed = ForwardMat4(State->View);
        Velocity = AddV3(Velocity, ForwardSpeed);
    }
    if(IsKeyDown('S'))
    {
        v3 BakcwardSpeed = BackwardMat4(State->View);
        Velocity = AddV3(Velocity, BakcwardSpeed);
    }
    if(IsKeyDown('Q'))
    {
        v3 LeftSpeed = LeftMat4(State->View);
        Velocity = AddV3(Velocity, LeftSpeed);
    }
    if(IsKeyDown('E'))
    {
        v3 RightSpeed = RightMat4(State->View);
        Velocity = AddV3(Velocity, RightSpeed);
    }
    if(IsKeyDown('R'))
    {
        Velocity.y += 1.0f;
    }
    if(IsKeyDown('F'))
    {
        Velocity.y -= 1.0f;
    }

    if(!EqV3(V3Zero(), Velocity, 0.0002f))
    {
        NormalizedV3(&Velocity);
        //State->CameraPosition = AddV3(MulV3(MulV3(Velocity, V3(TempMoveSpeed, TempMoveSpeed, TempMoveSpeed)), V3(DeltaTime, DeltaTime, DeltaTime)), State->CameraPosition);
        State->CameraPosition.x += Velocity.x * TempMoveSpeed * DeltaTime;
        State->CameraPosition.y += Velocity.y * TempMoveSpeed * DeltaTime;
        State->CameraPosition.z += Velocity.z * TempMoveSpeed * DeltaTime;
        State->CameraViewDirty = true;
    }

    RecalculateView(State);

    RendererSetView(&State->View);

    return true;
}

b8 Render(struct game* GameInst, r32 DeltaTime)
{
    return true;
}

void OnResize(struct game* GameInst, u32 Width, u32 Height)
{
}

