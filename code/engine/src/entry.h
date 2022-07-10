#include "core/application.h"
#include "core/logger.h"
#include "game_types.h"

extern b8 CreateGame(game* OutGame);

int main(void)
{
    game GameInstance;

    if(!CreateGame(&GameInstance))
    {
        VENG_FATAL("Could not create game instance");
        return -1;
    }

    if(!GameInstance.Render || !GameInstance.Update || !GameInstance.Initialize || !GameInstance.OnResize)
    {
        VENG_FATAL("The game function pointers must be assigned");
        return -2;
    }

    if(!ApplicationCreate(&GameInstance))
    {
        VENG_INFO("Application failed to create");
        return 1;
    }

    if(!ApplicationRun())
    {
        VENG_INFO("Application did not shutdown properly");
        return 2;
    }

    return 0;
}
