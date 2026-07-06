#include <stdio.h>

typedef struct MiniTask {
    const char *name;
    struct MiniTask *next;
} MiniTask;

typedef struct {
    const char *name;
    MiniTask *head;
} MiniList;

static void list_init(MiniList *list, const char *name) {
    list->name = name;
    list->head = 0;
    printf("%s: empty\n", list->name);
}

static void list_insert_front(MiniList *list, MiniTask *task) {
    task->next = list->head;
    list->head = task;
    printf("move %-6s -> %s\n", task->name, list->name);
}

static void list_remove(MiniList *list, MiniTask *task) {
    MiniTask **cursor = &list->head;
    while (*cursor) {
        if (*cursor == task) {
            *cursor = task->next;
            task->next = 0;
            printf("remove %-6s <- %s\n", task->name, list->name);
            return;
        }
        cursor = &(*cursor)->next;
    }
}

static void list_print(const MiniList *list) {
    const MiniTask *task = list->head;
    printf("%s:", list->name);
    if (!task) {
        printf(" <empty>");
    }
    while (task) {
        printf(" %s", task->name);
        task = task->next;
    }
    printf("\n");
}

int main(void) {
    MiniList ready;
    MiniList delayed;
    MiniList event_wait;
    MiniTask led = { "LED", 0 };
    MiniTask sensor = { "SENSOR", 0 };
    MiniTask log = { "LOG", 0 };

    list_init(&ready, "ready");
    list_init(&delayed, "delayed");
    list_init(&event_wait, "event_wait");
    list_insert_front(&ready, &led);
    list_insert_front(&ready, &sensor);
    list_insert_front(&ready, &log);
    list_print(&ready);

    list_remove(&ready, &sensor);
    list_insert_front(&delayed, &sensor);
    list_remove(&ready, &log);
    list_insert_front(&event_wait, &log);

    list_print(&ready);
    list_print(&delayed);
    list_print(&event_wait);
    return 0;
}

