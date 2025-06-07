#ifndef EMU_H
#define EMU_H

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stack.h"

#define MEMORY_SIZE               4096 // 4kB of RAM
#define NUM_OF_REGISTERS          16
#define FRAME_BUFFER_WIDTH_BYTES  64
#define FRAME_BUFFER_HEIGHT_BYTES 32

// Instructions
#define CLEAR_SCREEN_INSTR       0x0
#define JUMP_INSTR               0x1
#define SET_REGISTER_INSTR       0x6
#define ADD_VALUE_REGISTER_INSTR 0x7
#define SET_INDEX_REGISTER_INSTR 0xA
#define DRAW_INSTR               0xD

// Structures
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

typedef struct
{
    uint8_t  x;   // x offset from SCREEN_WIDTH
    uint8_t  y;   // y offset from SCREEN_HEIGHT
    uint8_t  vx;  // used to look up one of the 16 registers from V0 to FV
    uint8_t  vy;  // used to look up one of the 16 registers from V0 to FV
    uint8_t  n;   // the fourth nibble
    uint8_t  nn;  // the second byte
    uint16_t nnn; // the second, third and foirth nibbles
} OPCodeData;

// Function pointers
/** Emulator_ExecutionHandler 
 * Execute instruction decoded with Emulator_Decode
 * 
 * @param[in] OPCodeData pointer to data decoded from the instruction opcode.
 *                       It can contain register to take other data from,
 *                       x, y position at screen etc.
 * @param[in/out] MediaHandler framework handler needed to be read from/
 *                             be written to in order to provide graphics and media
 *                             utilities. Can be a structure holding multiple needed data.
 * @param[in/out] Emulator pointer to emulator structure
 *
*/
typedef void (*Emulator_ExecutionHandler)(const OPCodeData*, 
                                          void* MediaHandler,
                                          Emulator*);

// Extern variables
extern Emulator_ExecutionHandler clearScreenInstruction_FP;

// Function declarations
void Emulator_Init(Emulator**);
void Emulator_DeInit(Emulator**);
void Emulator_ClearMemory(Emulator*);
void Emulator_Reset(Emulator*);
void Emulator_LoadProgram(Emulator*, int);
uint16_t Emulator_Fetch(Emulator*);
uint8_t Emulator_Decode(Emulator*, const uint16_t, OPCodeData*);
Emulator_ExecutionHandler Emulator_Execute(Emulator*, const uint8_t);

#endif // EMU_H