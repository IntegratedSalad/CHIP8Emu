#ifndef EMUSDL_H
#define EMUSDL_H

#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "emu.h"

#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32
#define PIXEL_SCALE   10

typedef struct 
{
    SDL_Renderer* renderer;
    SDL_Window* window;
    // SDL_Surface* surface;
} SDL_App;

int SDL_App_Init(SDL_App**);
void SDL_App_DeInit(SDL_App**);
void SDL_App_Run(SDL_App*, Emulator*); // main loop for SDL implementation
void SDL_App_DrawXY(OPCodeData*, SDL_App*, Emulator*);
void SDL_App_DrawFrameBuffer(SDL_App* app_p, Emulator* emu_p);

#endif // EMUSDL_H