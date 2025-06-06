#include "emu.h"

void Emulator_Init(Emulator** emu_p)
{
    *emu_p = malloc(sizeof(Emulator));
    memset(*emu_p, 0, sizeof(Emulator));

    Stack_Init(&((*emu_p)->stack));
}

void Emulator_DeInit(Emulator** emu_p)
{
    Stack_DeInit(&(*emu_p)->stack);
    free(*emu_p);
    emu_p = NULL;
}

void Emulator_ClearMemory(Emulator* emu_p)
{
    
}

void Emulator_Reset(Emulator* emu_p)
{
    memset(emu_p, 0, sizeof(Emulator));
}
void Emulator_LoadProgram(Emulator*, const char*)
{

}

void Emulator_Run(Emulator*)
{

}