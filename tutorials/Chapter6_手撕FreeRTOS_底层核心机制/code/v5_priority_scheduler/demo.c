#include <stdio.h>

typedef enum { TASK_READY, TASK_BLOCKED } TaskState;

typedef struct {
    const char *name;
    unsigned priority;
    TaskState state;
} MiniTask;

static MiniTask *pick_next(MiniTask tasks[], unsigned count, unsigned *cursor) {
    MiniTask *best = 0;
    unsigned best_priority = 0;
    for (unsigned i = 0; i < count; ++i) {
        unsigned index = (*cursor + i) % count;
        MiniTask *task = &tasks[index];
        if (task->state != TASK_READY) {
            continue;
        }
        if (!best || task->priority > best_priority) {
            best = task;
            best_priority = task->priority;
        }
    }
    if (best) {
        *cursor = (unsigned)((best - tasks) + 1) % count;
    }
    return best;
}

int main(void) {
    MiniTask tasks[] = {
        { "LOG", 1, TASK_READY },
        { "LED", 2, TASK_READY },
        { "SENSOR", 2, TASK_READY },
        { "COMM", 3, TASK_BLOCKED }
    };
    unsigned cursor = 0;

    for (unsigned tick = 0; tick < 5; ++tick) {
        if (tick == 3) {
            tasks[3].state = TASK_READY;
            puts("event: COMM becomes ready");
        }
        MiniTask *next = pick_next(tasks, 4, &cursor);
        printf("tick=%u switch_to=%s priority=%u\n", tick, next->name, next->priority);
    }
    return 0;
}

