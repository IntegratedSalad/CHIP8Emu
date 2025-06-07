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

    memcpy(emu_p->memoryBuffer, programBuff, MEMORY_SIZE-1);
}

uint16_t Emulator_Fetch(Emulator* emu_p)
{
    const uint16_t pc = emu_p->PC;
    const uint16_t instr = (*(emu_p->memoryBuffer + pc)) | 
                           (*(emu_p->memoryBuffer + pc + 1));


    emu_p->PC += 2;
    return instr;
}