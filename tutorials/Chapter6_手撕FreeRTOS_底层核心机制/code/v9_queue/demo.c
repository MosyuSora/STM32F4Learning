#include <stdio.h>

#define QUEUE_CAPACITY 2

typedef struct {
    int buffer[QUEUE_CAPACITY];
    unsigned head;
    unsigned tail;
    unsigned count;
    const char *waiting_sender;
    const char *waiting_receiver;
} MiniQueue;

static int send(MiniQueue *queue, const char *task, int value) {
    if (queue->count == QUEUE_CAPACITY) {
        queue->waiting_sender = task;
        printf("%s send %d -> queue full, sender waits\n", task, value);
        return 0;
    }
    queue->buffer[queue->tail] = value;
    queue->tail = (queue->tail + 1) % QUEUE_CAPACITY;
    queue->count++;
    printf("%s send %d -> count=%u\n", task, value, queue->count);
    if (queue->waiting_receiver) {
        printf("wake receiver %s\n", queue->waiting_receiver);
        queue->waiting_receiver = 0;
    }
    return 1;
}

static int receive(MiniQueue *queue, const char *task, int *out) {
    if (queue->count == 0) {
        queue->waiting_receiver = task;
        printf("%s receive -> queue empty, receiver waits\n", task);
        return 0;
    }
    *out = queue->buffer[queue->head];
    queue->head = (queue->head + 1) % QUEUE_CAPACITY;
    queue->count--;
    printf("%s receive %d -> count=%u\n", task, *out, queue->count);
    if (queue->waiting_sender) {
        printf("wake sender %s\n", queue->waiting_sender);
        queue->waiting_sender = 0;
    }
    return 1;
}

int main(void) {
    MiniQueue queue = { { 0 }, 0, 0, 0, 0, 0 };
    int value = 0;

    receive(&queue, "LOG", &value);
    send(&queue, "COMM", 10);
    send(&queue, "COMM", 11);
    send(&queue, "COMM", 12);
    receive(&queue, "LOG", &value);
    send(&queue, "COMM", 12);
    return 0;
}

