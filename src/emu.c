#include "emu.h"

void Emulator_Init(Emulator** emu_p)
{
    *emu_p = malloc(sizeof(Emulator));
    memset(*emu_p, 0, sizeof(Emulator));

    (*emu_p)->PC = PROGRAM_MEMORY_OFFSET;
}

void Emulator_DeInit(Emulator** emu_p)
{
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

void Emulator_LoadProgram(Emulator* emu_p, int fd)
{
    char programBuff[MEMORY_SIZE];
    int nRead = 0;
    int totalRead = 0;
    programBuff[MEMORY_SIZE-1] = '\0';

    while ((nRead = read(fd, programBuff + totalRead, 1)) > 0)
    {
        totalRead += nRead;
    }

    memcpy(PROGRAM_MEMORY_OFFSET + emu_p->memoryBuffer, programBuff, MEMORY_SIZE-1);
}

uint16_t Emulator_Fetch(Emulator* emu_p)
{
    const uint16_t pc = emu_p->PC;
    const uint16_t instr = (*(emu_p->memoryBuffer + pc)) << 8 | 
                           (*(emu_p->memoryBuffer + pc + 1));

    emu_p->PC += 2;
    return instr;
}

uint8_t Emulator_Decode(Emulator* emu_p, const uint16_t instr, OPCodeData* opcodeData_p)
{
    // TODO: INTERPRET EVERY NUMBER AS HEXADECIMAL!
    const uint8_t type = instr >> 12; // 4 bit 'identifier'
    // fill the entire opcodeData
    opcodeData_p->vx =   (instr >> 8) & (0xF);   // second nibble
    opcodeData_p->vy =   (instr >> 4) & (0xF);   // third nibble 
    opcodeData_p->n =     instr       & (0xF);   // fourth nibble
    opcodeData_p->nn =    instr       & (0xFF);  // the second byte
    opcodeData_p->nnn =   instr       & (0xFFF);
    opcodeData_p->x =    (instr >> 8) & (0xF);   // second nibble, in display/draw instruction
    opcodeData_p->y =    (instr >> 4) & (0xF);   // third nibble, in display/draw instruction

    return type;
}

void Emulator_Jump(OPCodeData* opcodeData_p, 
                   void* MediaHandler, 
                   Emulator* emu_p)
{
    // printf("Jump location: 0x%x\n", opcodeData_p->nnn);
    // printf("Jump location: %d\n", opcodeData_p->nnn);
    // printf("Jump location-offset: %d\n", opcodeData_p->nnn - PROGRAM_MEMORY_OFFSET);
    emu_p->PC = opcodeData_p->nnn;
}

void Emulator_ClearScreen(OPCodeData* opcodeData_p, 
                          void* MediaHandler, 
                          Emulator* emu_p)
{
    memset(emu_p->framebuffer, 0, sizeof(emu_p->framebuffer));
}

void Emulator_SetIndexRegister(OPCodeData* opcodeData_p, 
                               void* MediaHandler, 
                               Emulator* emu_p)
{
    emu_p->indexRegister = opcodeData_p->nnn;
}

void Emulator_SetRegisterVX(OPCodeData* opcodeData_p, 
                            void* MediaHandler, 
                            Emulator* emu_p)
{
    const int8_t registerNum = opcodeData_p->vx;
    emu_p->registerArray[registerNum] = opcodeData_p->nn;
}

void Emulator_AddValueToRegisterVX(OPCodeData* opcodeData_p, 
                                   void* MediaHandler, 
                                   Emulator* emu_p)
{
    const int8_t registerNum = opcodeData_p->vx;
    emu_p->registerArray[registerNum] += opcodeData_p->nn;
}

void Emulator_JumpWithOffset(OPCodeData* opcodeData_p, 
                             void* MediaHandler, 
                             Emulator* emu_p)
{
    const int8_t registerNum = opcodeData_p->vx;
    const int8_t offset = emu_p->registerArray[registerNum];
    emu_p->PC = opcodeData_p->nnn + offset;
}

// Function pointers assignment for platform independent implementations
Emulator_ExecutionHandler jumpInstruction_FP = &Emulator_Jump;
Emulator_ExecutionHandler clearScreen_Instruction_FP = &Emulator_ClearScreen;
Emulator_ExecutionHandler setIndexRegisterVXInstruction_FP = &Emulator_SetIndexRegister;
Emulator_ExecutionHandler setRegisterVXInstruction_FP = &Emulator_SetRegisterVX;
Emulator_ExecutionHandler addValueToRegisterVXInstruction_FP = &Emulator_AddValueToRegisterVX;
Emulator_ExecutionHandler jumpWithOffsetInstruction_FP = &Emulator_JumpWithOffset;

// Map executionHandler to the instruction type
Emulator_ExecutionHandler Emulator_MapExecutionHandler(const uint8_t type)
{
    Emulator_ExecutionHandler execHandler = NULL;
    switch (type)
    {
        case CLEAR_SCREEN_INSTR:
        {
            execHandler = clearScreen_Instruction_FP;
            break;
        }
        case DRAW_INSTR:
        {
            execHandler = drawPixelsToScreenInstruction_FP;
            break;
        }
        case JUMP_INSTR:
        {
            execHandler = jumpInstruction_FP;
            break;
        }
        case SET_REGISTER_INSTR:
        {
            execHandler = setRegisterVXInstruction_FP;
            break;
        }
        case ADD_VALUE_REGISTER_INSTR:
        {
            execHandler = addValueToRegisterVXInstruction_FP;
            break;
        }
        case SET_INDEX_REGISTER_INSTR:
        {
            execHandler = setIndexRegisterVXInstruction_FP;
            break;
        }
        case JUMP_WITH_OFFSET_INSTR:
        {
            execHandler = jumpWithOffsetInstruction_FP;
        }
        default:
        {
            break;
        }
    }
    return execHandler;
}
