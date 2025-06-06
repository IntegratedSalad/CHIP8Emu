#include "stack.h"

void Stack_Init(Stack** stack_p)
{
    *stack_p = malloc(sizeof(stack_p));
    memset(*stack_p, 0, sizeof(Stack));
}

void Stack_DeInit(Stack** stack)
{
    free(*stack);
    stack = NULL;
}

int Stack_Push(Stack*)
{
    return -1;
}

int Stack_Pop(Stack*)
{
    return -1;
}

int Stack_Clear(Stack*)
{
    return -1;
}