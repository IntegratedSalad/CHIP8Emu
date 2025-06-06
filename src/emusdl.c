#include "emusdl.h"

int SDL_App_Init(SDL_App** app_pp)
{
    int error = 0;

    *app_pp = malloc(sizeof(SDL_App));
    memset(*app_pp, 0, sizeof(SDL_App));

    SDL_App* app_p = *app_pp;

    app_p->window = NULL;
    app_p->renderer = NULL;
    app_p->surface = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL Couldn't be initialized. SDL_Error: %s\n", SDL_GetError());
        error = 1;
    } else {
        app_p->window = SDL_CreateWindow("Chip8 EmuSDL", 
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SCREEN_WIDTH * PIXEL_SCALE,
                                         SCREEN_HEIGHT * PIXEL_SCALE,
                                         SDL_WINDOW_SHOWN
                                        );
        if (app_p->window == NULL)
        {
            fprintf(stderr, "Window couldn't be created. SDL_Error: %s\n", SDL_GetError());
            error = 1;
        } else
        {
            app_p->surface = SDL_GetWindowSurface(app_p->window);
            SDL_FillRect(app_p->surface, NULL, SDL_MapRGB(app_p->surface->format, 0x0, 0x0, 0x0));
            SDL_UpdateWindowSurface(app_p->window);
        }
    }

    return error;
}

void SDL_App_DeInit(SDL_App** app_p)
{
    SDL_DestroyWindow((*app_p)->window);
    SDL_Quit();

    free(*app_p);
    app_p = NULL;
}

void SDL_App_Emulate(SDL_App* app_p, Emulator* emu_p)
{
    SDL_Event e;
    int running = 1;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
        }
    }
}