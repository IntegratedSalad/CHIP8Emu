#pragma once
#include <SDL.h>
#include <stdlib.h>
#include "emu.h"

#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32

typedef struct 
{
    SDL_Renderer* renderer;
    SDL_Window* window;
} SDL_App;

void SDL_Init(SDL_App*);
void SDL_DeInit(SDL_App*);
void SDL_Emulate(SDL_App*, Emulator*); // main loop for SDL implementation