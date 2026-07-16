#include <stdint.h>
#include <stdio.h>

typedef struct {
    const char *name;
    uint32_t process_stack_pointer;
} MiniTCB;

static MiniTCB *pxCurrentTCB;

static void pendsv_switch(MiniTCB *next) {
    printf("PendSV: save PSP=0x%08lx into %s TCB\n",
           (unsigned long)pxCurrentTCB->process_stack_pointer,
           pxCurrentTCB->name);
    printf("PendSV: pxCurrentTCB %s -> %s\n", pxCurrentTCB->name, next->name);
    pxCurrentTCB = next;
    printf("PendSV: restore PSP=0x%08lx from %s TCB\n",
           (unsigned long)pxCurrentTCB->process_stack_pointer,
           pxCurrentTCB->name);
}

int main(void) {
    MiniTCB led = { "LED", 0x20001000u };
    MiniTCB log = { "LOG", 0x20002000u };
    pxCurrentTCB = &led;

    puts("model: scheduler already selected LOG");
    pendsv_switch(&log);
    return 0;
}

