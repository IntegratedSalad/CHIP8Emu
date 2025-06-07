#pragma once
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stack.h"

#define MEMORY_SIZE               4096 // 4kB of RAM
#define NUM_OF_REGISTERS          16
#define FRAME_BUFFER_WIDTH_BYTES  64
#define FRAME_BUFFER_HEIGHT_BYTES 32

// #define 

typedef struct
{
    uint8_t   memoryBuffer[MEMORY_SIZE]; // RAM
    uint8_t   registerArray[NUM_OF_REGISTERS];
    uint16_t  indexRegister;
    Stack*    stack;
    uint8_t   framebuffer[FRAME_BUFFER_WIDTH_BYTES][FRAME_BUFFER_HEIGHT_BYTES];
    uint8_t   delayTimer;
    uint8_t   soundTimer;
    uint16_t  PC;
} Emulator;

void Emulator_Init(Emulator**);
void Emulator_DeInit(Emulator**);
void Emulator_ClearMemory(Emulator*);
void Emulator_Reset(Emulator*);
void Emulator_LoadProgram(Emulator*, int);
uint16_t Emulator_Fetch(Emulator*);
void Emulator_Decode(Emulator*);
void Emulator_Execute(Emulator*);
