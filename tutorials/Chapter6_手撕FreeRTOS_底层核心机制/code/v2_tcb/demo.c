#include <stdint.h>
#include <stdio.h>

typedef struct MiniListItem {
    const char *owner_name;
    struct MiniListItem *next;
} MiniListItem;

typedef struct {
    const char *name;
    uint32_t *top_of_stack;
    unsigned priority;
    MiniListItem state_item;
} MiniTCB;

static void print_tcb(const MiniTCB *tcb) {
    printf("TCB name=%-7s priority=%u top_of_stack=%p list_owner=%s\n",
           tcb->name,
           tcb->priority,
           (void *)tcb->top_of_stack,
           tcb->state_item.owner_name);
}

int main(void) {
    uint32_t led_stack[8];
    uint32_t log_stack[8];

    MiniTCB led = { "LED", &led_stack[7], 2, { "LED", 0 } };
    MiniTCB log = { "LOG", &log_stack[7], 1, { "LOG", 0 } };

    print_tcb(&led);
    print_tcb(&log);
    puts("TCB is the scheduler handle: identity + stack + priority + list hook");
    return 0;
}

