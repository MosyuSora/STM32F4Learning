#include <stdint.h>
#include <stdio.h>

typedef void (*TaskEntry)(void *);

typedef struct {
    const char *name;
    TaskEntry entry;
    void *parameter;
    uint32_t stack[8];
    uint32_t *top_of_stack;
} MiniTaskStack;

static void led_task(void *parameter) {
    printf("run task=LED parameter=%s\n", (const char *)parameter);
}

static void sensor_task(void *parameter) {
    printf("run task=SENSOR period_ms=%d\n", *(int *)parameter);
}

static void initialise_stack(MiniTaskStack *task, const char *name, TaskEntry entry, void *parameter) {
    task->name = name;
    task->entry = entry;
    task->parameter = parameter;
    task->top_of_stack = &task->stack[7];
    task->stack[7] = (uint32_t)(uintptr_t)entry;
    task->stack[6] = (uint32_t)(uintptr_t)parameter;
    printf("init %-6s stack_base=%p top=%p entry_slot=0x%08lx parameter_slot=0x%08lx\n",
           task->name,
           (void *)&task->stack[0],
           (void *)task->top_of_stack,
           (unsigned long)task->stack[7],
           (unsigned long)task->stack[6]);
}

int main(void) {
    MiniTaskStack led;
    MiniTaskStack sensor;
    int sensor_period = 20;

    initialise_stack(&led, "LED", led_task, "heartbeat");
    initialise_stack(&sensor, "SENSOR", sensor_task, &sensor_period);

    puts("scheduler restores LED stack");
    led.entry(led.parameter);
    puts("scheduler restores SENSOR stack");
    sensor.entry(sensor.parameter);
    return 0;
}

