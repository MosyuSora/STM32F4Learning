#include <stdio.h>

typedef struct {
    const char *name;
    unsigned base_priority;
    unsigned current_priority;
    int blocked;
} MiniTask;

typedef struct {
    MiniTask *owner;
} MiniMutex;

static void take_mutex(MiniMutex *mutex, MiniTask *task) {
    if (!mutex->owner) {
        mutex->owner = task;
        printf("%s takes mutex\n", task->name);
        return;
    }
    task->blocked = 1;
    printf("%s waits for mutex owned by %s\n", task->name, mutex->owner->name);
    if (task->current_priority > mutex->owner->current_priority) {
        printf("inherit: %s priority %u -> %u\n",
               mutex->owner->name,
               mutex->owner->current_priority,
               task->current_priority);
        mutex->owner->current_priority = task->current_priority;
    }
}

static void give_mutex(MiniMutex *mutex) {
    MiniTask *owner = mutex->owner;
    printf("%s releases mutex, priority restores %u -> %u\n",
           owner->name,
           owner->current_priority,
           owner->base_priority);
    owner->current_priority = owner->base_priority;
    mutex->owner = 0;
}

int main(void) {
    MiniTask low = { "LOW_LOG", 1, 1, 0 };
    MiniTask mid = { "MID_WORK", 2, 2, 0 };
    MiniTask high = { "HIGH_COMM", 3, 3, 0 };
    MiniMutex bus = { 0 };

    take_mutex(&bus, &low);
    printf("%s is ready and could preempt LOW_LOG if no inheritance exists\n", mid.name);
    take_mutex(&bus, &high);
    give_mutex(&bus);
    return 0;
}

