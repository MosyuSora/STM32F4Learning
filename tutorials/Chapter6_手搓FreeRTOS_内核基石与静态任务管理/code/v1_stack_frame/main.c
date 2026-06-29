#include <stdint.h>
#include <stdio.h>

typedef uintptr_t StackType_t;
typedef void (*TaskFunction_t)(void *);

#define STACK_WORDS 64u
#define INITIAL_XPSR ((StackType_t)0x01000000u)
#define TASK_RETURN_ADDRESS ((StackType_t)0xDEADBEEFu)
#define INITIAL_EXC_RETURN ((StackType_t)0xFFFFFFFDu)

static void led_task(void *arg)
{
    (void)arg;
}

static StackType_t *pxPortInitialiseStack(StackType_t *top,
                                          TaskFunction_t entry,
                                          void *parameter)
{
    *--top = INITIAL_XPSR;
    *--top = (StackType_t)entry;
    *--top = TASK_RETURN_ADDRESS;
    *--top = (StackType_t)0x12121212u;
    *--top = (StackType_t)0x03030303u;
    *--top = (StackType_t)0x02020202u;
    *--top = (StackType_t)0x01010101u;
    *--top = (StackType_t)parameter;
    *--top = INITIAL_EXC_RETURN;

    for (int reg = 11; reg >= 4; --reg) {
        *--top = (StackType_t)(0x44440000u + (unsigned)reg);
    }

    return top;
}

static void dump_stack_frame(StackType_t *sp)
{
    const char *names[] = {
        "R4 ", "R5 ", "R6 ", "R7 ", "R8 ", "R9 ", "R10", "R11",
        "EXC_RETURN",
        "R0 ", "R1 ", "R2 ", "R3 ", "R12", "LR ", "PC ", "xPSR"
    };

    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        printf("  sp[%02zu] %-10s = 0x%0*llX\n",
               i,
               names[i],
               (int)(sizeof(StackType_t) * 2),
               (unsigned long long)sp[i]);
    }
}

int main(void)
{
    int led_parameter = 0x12345678;
    StackType_t task_stack[STACK_WORDS] = {0};
    StackType_t *top = task_stack + STACK_WORDS;
    StackType_t *sp = pxPortInitialiseStack(top, led_task, &led_parameter);

    printf("Ch6 v1: build an initial task stack frame\n");
    printf("stack base=%p top=%p initial sp=%p\n\n",
           (void *)task_stack, (void *)top, (void *)sp);
    dump_stack_frame(sp);

    printf("\nThe fake PC points to led_task: %p\n", (void *)led_task);
    printf("The fake LR points to the task-return guard: 0x%0*llX\n",
           (int)(sizeof(StackType_t) * 2),
           (unsigned long long)TASK_RETURN_ADDRESS);
    printf("The saved EXC_RETURN asks Cortex-M to return to Thread mode with PSP: 0x%08X\n",
           (unsigned)INITIAL_EXC_RETURN);
    printf("The fake xPSR keeps the Thumb bit set: 0x%08X\n", 0x01000000u);
    return 0;
}
