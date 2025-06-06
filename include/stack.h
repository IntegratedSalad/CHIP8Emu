#include <stdlib.h>

#define STACK_SIZE 8 * 8

typedef struct
{
    uint8_t stackArray[STACK_SIZE];
    uint8_t SP;
} Stack;

void Stack_Init(Stack*);
void Stack_Deinit(Stack*);
int Stack_Push(Stack*);
int Stack_Pop(Stack*);
int Stack_Clear(Stack*);