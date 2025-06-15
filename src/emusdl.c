#include "emusdl.h"

int SDL_App_Init(SDL_App** app_pp)
{
    int error = 0;

    *app_pp = malloc(sizeof(SDL_App));
    memset(*app_pp, 0, sizeof(SDL_App));

    SDL_App* app_p = *app_pp;

    app_p->window = NULL;
    app_p->renderer = NULL;
    // app_p->surface = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL Couldn't be initialized. SDL_Error: %s\n", SDL_GetError());
        error = 1;
    } else
    {
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
        } else {
            app_p->renderer = SDL_CreateRenderer(app_p->window, -1, SDL_RENDERER_ACCELERATED);
            if (app_p->renderer == NULL)
            {
                fprintf(stderr, "Renderer couldn't be created. SDL_Error: %s\n", SDL_GetError());
                error = 1; 
            } else {
                SDL_SetRenderDrawColor(app_p->renderer, 0, 0, 0, 255);
                SDL_RenderClear(app_p->renderer);
                SDL_RenderPresent(app_p->renderer);
            }
        }
    }
    return error;
}

void SDL_App_DeInit(SDL_App** app_p)
{
    SDL_DestroyRenderer((*app_p)->renderer);
    SDL_DestroyWindow((*app_p)->window);
    SDL_Quit();

    free(*app_p);
    app_p = NULL;
}

// TODO: setting memory buffer should be abstracted to the implementation-independent routine
void SDL_App_DrawXY(OPCodeData* opcodeData_p, 
                    SDL_App* app_p, 
                    Emulator* emu_p)
{
    const uint8_t registerVXNum = opcodeData_p->vx;
    const uint8_t registerVYNum = opcodeData_p->vy;
    uint8_t y = emu_p->registerArray[registerVYNum] % 32;

    const uint8_t x = (emu_p->registerArray[registerVXNum] % 64);
    const uint8_t bitX  = x%8;
    const uint8_t byteX = x/8;
    uint8_t byteOverflow = 0;

    const uint8_t nBytes = opcodeData_p->n;
    emu_p->registerArray[0xF] = 0;
    uint16_t iRegisterAddress = emu_p->indexRegister;

    for (int8_t nByte = 0; nByte < nBytes; nByte++)
    {
        const uint8_t byteToDraw = *(emu_p->memoryBuffer + iRegisterAddress + nByte);
        for (int8_t bit = 7; bit >= 0; bit--)
        {
            if ((x + bit) == SCREEN_WIDTH - 1)
            {
                break;
            }
            // XOR the BYTE at fb[y][x], bit by bit

            byteOverflow = (bitX + (7 - bit)) / 8; 
            
            // If it goes to the next byte in fb, it has to start
            // from MSB in that byte! byteToDraw has to continue as it were...
            if (byteOverflow == 0)
            {
                emu_p->framebuffer[y][byteX] ^= ((byteToDraw & (0x1 << bit))) >> bitX;
            } else 
            {
                emu_p->framebuffer[y][byteX + byteOverflow] ^= ((byteToDraw & (0x1 << bit))) << (8 - bitX);
            }
        }
        y++; // increment the BIT
        if (y == SCREEN_HEIGHT - 1)
        {
            break;
        }
    }
}

void SDL_App_DrawFrameBuffer(SDL_App* app_p, Emulator* emu_p)
{
    SDL_SetRenderDrawColor(app_p->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    uint8_t x = 0;
    for (uint8_t fbH = 0; fbH < FRAME_BUFFER_HEIGHT_BYTES; fbH++)
    {
        for (uint8_t fbW = 0; fbW < FRAME_BUFFER_WIDTH_BYTES; fbW++)
        {
            const uint8_t byteToDraw = emu_p->framebuffer[fbH][fbW];
            for (int8_t bit = 7; bit >= 0; bit--)
            {
                /* Select Nth byte and draw it from MSB */
                x = (7 - bit) + (8 * fbW);
                if (byteToDraw & (0x1 << bit))
                {
                    SDL_Rect r = {.x = x * 10, .y = fbH * 10, .w = 10, .h = 10};
                    SDL_RenderFillRect(app_p->renderer, &r);
                    SDL_RenderDrawRect(app_p->renderer, &r);
                }
            }
        }
        x = 0;
    }
}

void SDL_App_Run(SDL_App* app_p, Emulator* emu_p)
{
    SDL_Event e;
    int running = 1;

    OPCodeData opcodeData;
    uint16_t currentInstructionCode = 0;
    // currentInstructionCode = Emulator_Fetch(emu_p);
    Emulator_ExecutionHandler execHandler = NULL;
    uint8_t instructionType;// = Emulator_Decode(emu_p, currentInstructionCode, &opcodeData);

    // execHandler = Emulator_MapExecutionHandler(instructionType);
    // execHandler(&opcodeData, app_p, emu_p);

    uint32_t ticks = 0;
    uint32_t delta = 0;
    while (running)
    {
        ticks = SDL_GetTicks();
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(app_p->renderer, 0x0, 0x0, 0x0, 0xFF);
        SDL_RenderClear(app_p->renderer);
        currentInstructionCode = Emulator_Fetch(emu_p);
        printf("Current instruction: 0x%x\n", currentInstructionCode);
        instructionType = Emulator_Decode(emu_p, currentInstructionCode, &opcodeData);
        execHandler = Emulator_MapExecutionHandler(instructionType);
        execHandler(&opcodeData, app_p, emu_p);

        SDL_App_DrawFrameBuffer(app_p, emu_p);
        SDL_RenderPresent(app_p->renderer);

        delta = SDL_GetTicks() - ticks;
        if (1000/60.0 > delta)
        {
            SDL_Delay(1000/60.0 - delta);
        }
    }
}

// Function pointers assignment for implementation dependent function handlers
Emulator_ExecutionHandler drawPixelsToScreenInstruction_FP = &SDL_App_DrawXY;