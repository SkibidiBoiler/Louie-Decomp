// Entry point: SDL3 main callbacks, shared by Windows and Android.
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "engine/app.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    auto *app = new engine::App();
    *appstate = app;
    return app->init(argc, argv);
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    return static_cast<engine::App *>(appstate)->event(*event);
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    return static_cast<engine::App *>(appstate)->iterate();
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;
    delete static_cast<engine::App *>(appstate);
}
