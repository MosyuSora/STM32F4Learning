#include <stdio.h>

typedef struct {
    const char *name;
    unsigned priority;
} MiniTask;

static MiniTask *select_first_task(MiniTask tasks[], unsigned count) {
    MiniTask *best = &tasks[0];
    for (unsigned i = 1; i < count; ++i) {
        if (tasks[i].priority > best->priority) {
            best = &tasks[i];
        }
    }
    return best;
}

static void start_first_task(MiniTask *task) {
    puts("SVC model: restore the prepared first task context");
    printf("first task=%s priority=%u\n", task->name, task->priority);
    puts("main stops owning CPU; task context owns execution");
}

int main(void) {
    MiniTask tasks[] = {
        { "LED", 2 },
        { "LOG", 1 },
        { "COMM", 3 }
    };
    puts("main: create tasks and start scheduler");
    start_first_task(select_first_task(tasks, 3));
    return 0;
}

