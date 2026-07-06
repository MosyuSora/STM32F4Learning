#include <stdint.h>
#include <stdio.h>

typedef void (*TaskEntry)(void *);

typedef struct MiniTCB {
    const char *name;
    TaskEntry entry;
    void *parameter;
    uint32_t *top_of_stack;
    unsigned priority;
    struct MiniTCB *next_ready;
} MiniTCB;

typedef struct {
    MiniTCB *head;
} ReadyList;

static void ready_insert(ReadyList *ready, MiniTCB *task) {
    task->next_ready = ready->head;
    ready->head = task;
}

static MiniTCB *mini_xTaskCreateStatic(TaskEntry entry,
                                       const char *name,
                                       void *parameter,
                                       unsigned priority,
                                       uint32_t *stack_base,
                                       unsigned stack_words,
                                       MiniTCB *tcb,
                                       ReadyList *ready) {
    tcb->name = name;
    tcb->entry = entry;
    tcb->parameter = parameter;
    tcb->priority = priority;
    tcb->top_of_stack = &stack_base[stack_words - 1];
    ready_insert(ready, tcb);
    printf("created %-6s priority=%u top=%p -> ready list\n",
           tcb->name,
           tcb->priority,
           (void *)tcb->top_of_stack);
    return tcb;
}

static void task_entry(void *parameter) {
    printf("task would run with parameter=%s\n", (const char *)parameter);
}

int main(void) {
    ReadyList ready = { 0 };
    uint32_t led_stack[8];
    uint32_t log_stack[8];
    MiniTCB led_tcb;
    MiniTCB log_tcb;

    mini_xTaskCreateStatic(task_entry, "LED", "heartbeat", 2, led_stack, 8, &led_tcb, &ready);
    mini_xTaskCreateStatic(task_entry, "LOG", "uart", 1, log_stack, 8, &log_tcb, &ready);

    puts("ready list after creation:");
    for (MiniTCB *task = ready.head; task; task = task->next_ready) {
        printf("  %s is ready but not necessarily running\n", task->name);
    }
    return 0;
}

