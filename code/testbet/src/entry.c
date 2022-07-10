
#include "game.h"

#include <entry.h>
#include <core/vmemory.h>

b8 CreateGame(game* OutGame)
{
    OutGame->AppConfig.StartX = 100;
    OutGame->AppConfig.StartY = 100;
    OutGame->AppConfig.StartWidth = 1240;
    OutGame->AppConfig.StartHeight = 720;
    OutGame->AppConfig.Name = "VENG Engine Application";

    OutGame->Update = Update;
    OutGame->Render = Render;
    OutGame->Initialize = Initialize;
    OutGame->OnResize = OnResize;
    
    OutGame->ApplicationState = 0;

    OutGame->State = Allocate(sizeof(game_state), MEMORY_TAG_GAME);

    return true;
}
