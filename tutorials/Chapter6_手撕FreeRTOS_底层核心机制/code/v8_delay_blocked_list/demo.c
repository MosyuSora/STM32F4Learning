#include <stdio.h>

typedef enum { READY, DELAYED } State;

typedef struct {
    const char *name;
    State state;
    unsigned wake_tick;
} MiniTask;

static void mini_delay(MiniTask *task, unsigned now, unsigned ticks_to_delay) {
    task->state = DELAYED;
    task->wake_tick = now + ticks_to_delay;
    printf("tick=%u %s: ready -> delayed until tick %u\n", now, task->name, task->wake_tick);
}

static void tick(MiniTask tasks[], unsigned count, unsigned now) {
    printf("tick=%u sys tick\n", now);
    for (unsigned i = 0; i < count; ++i) {
        if (tasks[i].state == DELAYED && now >= tasks[i].wake_tick) {
            tasks[i].state = READY;
            printf("tick=%u %s: delayed -> ready, not necessarily running yet\n", now, tasks[i].name);
        }
    }
}

int main(void) {
    MiniTask tasks[] = {
        { "LED", READY, 0 },
        { "SENSOR", READY, 0 }
    };

    mini_delay(&tasks[0], 0, 3);
    for (unsigned now = 1; now <= 4; ++now) {
        tick(tasks, 2, now);
    }
    return 0;
}

