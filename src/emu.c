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

    memcpy(PROGRAM_MEMORY_OFFSET + emu_p->memoryBuffer, programBuff, MEMORY_SIZE - PROGRAM_MEMORY_OFFSET);
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
    const uint8_t type = instr >> 12; // 4 bit 'identifier'

    // TODO: Decode logical and arithmetic instructions and subroutines

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

// OPCODE: 0 0E0
void Emulator_ClearScreen(OPCodeData* opcodeData_p, 
                          void* MediaHandler, 
                          Emulator* emu_p)
{
    memset(emu_p->framebuffer_p, 0, sizeof(emu_p->framebuffer_p));
}

// OPCODE: 1 NNN
void Emulator_Jump(OPCodeData* opcodeData_p, 
                   void* MediaHandler, 
                   Emulator* emu_p)
{
    emu_p->PC = opcodeData_p->nnn;
}

// OPCODE: A NNN
void Emulator_SetIndexRegister(OPCodeData* opcodeData_p, 
                               void* MediaHandler, 
                               Emulator* emu_p)
{
    emu_p->indexRegister = opcodeData_p->nnn;
}

// OPCODE: 6 XNN
void Emulator_SetRegisterVX(OPCodeData* opcodeData_p, 
                            void* MediaHandler, 
                            Emulator* emu_p)
{
    const int8_t registerNum = opcodeData_p->vx;
    emu_p->registerArray[registerNum] = opcodeData_p->nn;
}

// OPCODE: 7 XNN
void Emulator_AddValueToRegisterVX(OPCodeData* opcodeData_p, 
                                   void* MediaHandler, 
                                   Emulator* emu_p)
{
    const int8_t registerNum = opcodeData_p->vx;
    emu_p->registerArray[registerNum] += opcodeData_p->nn;
}

// OPCODE: B NNN
void Emulator_JumpWithOffset(OPCodeData* opcodeData_p, 
                             void* MediaHandler, 
                             Emulator* emu_p)
{
    const int8_t registerNum = opcodeData_p->vx;
    const int8_t offset = emu_p->registerArray[registerNum];
    emu_p->PC = opcodeData_p->nnn + offset;
}

// OPCODE: D XYN
void Emulator_CopyVideoDataToFrameBuffer(OPCodeData* opcodeData_p, 
                                        void* MediaHandler, 
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

            byteOverflow = (bitX + (7 - bit)) / 8; 
            
            // If it goes to the next byte in fb, it has to start
            // from MSB in that byte! byteToDraw has to continue as it was...
            // XOR the BYTE at fb[y][x], bit by bit
            if (byteOverflow == 0)
            {
                emu_p->framebuffer_p[y][byteX] ^= ((byteToDraw & (0x1 << bit))) >> bitX;
            } else 
            {
                emu_p->framebuffer_p[y][byteX + byteOverflow] ^= ((byteToDraw & (0x1 << bit))) << (8 - bitX);
            }
        }
        y++; // increment the BIT
        if (y == SCREEN_HEIGHT - 1)
        {
            break;
        }
    }
}

// Function pointers assignment for platform independent implementations
Emulator_ExecutionHandler jumpInstruction_FP = &Emulator_Jump;
Emulator_ExecutionHandler clearScreen_Instruction_FP = &Emulator_ClearScreen;
Emulator_ExecutionHandler setIndexRegisterVXInstruction_FP = &Emulator_SetIndexRegister;
Emulator_ExecutionHandler setRegisterVXInstruction_FP = &Emulator_SetRegisterVX;
Emulator_ExecutionHandler addValueToRegisterVXInstruction_FP = &Emulator_AddValueToRegisterVX;
Emulator_ExecutionHandler jumpWithOffsetInstruction_FP = &Emulator_JumpWithOffset;
Emulator_ExecutionHandler drawPixelsToScreenInstruction_FP = &Emulator_CopyVideoDataToFrameBuffer;

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
            break;
        }
        default:
        {
            break;
        }
    }
    return execHandler;
}
