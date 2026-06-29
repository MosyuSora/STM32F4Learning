#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

typedef uintptr_t StackType_t;
typedef unsigned UBaseType_t;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *pvOwner;
} ListNode_t;

typedef struct {
    StackType_t *pxTopOfStack;
    ListNode_t state_node;
    UBaseType_t uxPriority;
    const char *pcTaskName;
} TCB_t;

#define TCB_FROM_STATE_NODE(ptr) container_of(ptr, TCB_t, state_node)

static void list_init(ListNode_t *head)
{
    head->next = head;
    head->prev = head;
    head->pvOwner = NULL;
}

static void list_insert_tail(ListNode_t *head, ListNode_t *node)
{
    node->next = head;
    node->prev = head->prev;
    head->prev->next = node;
    head->prev = node;
}

static void tcb_init(TCB_t *tcb, const char *name, UBaseType_t priority)
{
    tcb->pxTopOfStack = NULL;
    tcb->state_node.next = NULL;
    tcb->state_node.prev = NULL;
    tcb->state_node.pvOwner = tcb;
    tcb->uxPriority = priority;
    tcb->pcTaskName = name;
}

int main(void)
{
    ListNode_t ready_list;
    TCB_t led_task;
    TCB_t uart_task;
    TCB_t sensor_task;

    list_init(&ready_list);
    tcb_init(&led_task, "LED", 1);
    tcb_init(&uart_task, "UART", 2);
    tcb_init(&sensor_task, "SENSOR", 1);

    list_insert_tail(&ready_list, &led_task.state_node);
    list_insert_tail(&ready_list, &uart_task.state_node);
    list_insert_tail(&ready_list, &sensor_task.state_node);

    printf("Ch6 v2: ready list stores nodes, scheduler recovers TCB\n\n");
    for (ListNode_t *iter = ready_list.next; iter != &ready_list; iter = iter->next) {
        TCB_t *by_container = TCB_FROM_STATE_NODE(iter);
        TCB_t *by_owner = (TCB_t *)iter->pvOwner;

        printf("  node=%p -> container_of=%p owner=%p name=%s priority=%u\n",
               (void *)iter,
               (void *)by_container,
               (void *)by_owner,
               by_container->pcTaskName,
               by_container->uxPriority);
    }

    printf("\noffsetof(TCB_t, state_node) = %zu\n", offsetof(TCB_t, state_node));
    return 0;
}
