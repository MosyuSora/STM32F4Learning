#include <stdint.h>
#include <stdio.h>

#define portNVIC_PENDSVSET_BIT (1u << 28)

typedef uintptr_t StackType_t;
typedef void (*TaskStep_t)(void *);

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *pvOwner;
} ListNode_t;

typedef struct TCB {
    StackType_t *pxTopOfStack;
    ListNode_t xStateListItem;
    TaskStep_t step;
    void *parameter;
    const char *pcTaskName;
    unsigned runs;
} TCB_t;

static ListNode_t ready_list;
static TCB_t *pxCurrentTCB;
static uint32_t simulated_icsr;

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

static TCB_t *task_from_node(ListNode_t *node)
{
    return (TCB_t *)node->pvOwner;
}

static void tcb_init(TCB_t *tcb, TaskStep_t step, const char *name)
{
    tcb->pxTopOfStack = NULL;
    tcb->xStateListItem.next = NULL;
    tcb->xStateListItem.prev = NULL;
    tcb->xStateListItem.pvOwner = tcb;
    tcb->step = step;
    tcb->parameter = NULL;
    tcb->pcTaskName = name;
    tcb->runs = 0;
}

static void prvSelectNextTask(void)
{
    ListNode_t *next = pxCurrentTCB->xStateListItem.next;
    if (next == &ready_list) {
        next = ready_list.next;
    }
    pxCurrentTCB = task_from_node(next);
}

static void xPortPendSVHandler(void)
{
    printf("  [PendSV] save %s R4-R11 to its stack\n", pxCurrentTCB->pcTaskName);
    prvSelectNextTask();
    printf("  [PendSV] restore %s R4-R11 from its stack\n", pxCurrentTCB->pcTaskName);
}

static void portYIELD(void)
{
    simulated_icsr = portNVIC_PENDSVSET_BIT;
    printf("  [%s] taskYIELD -> set PENDSVSET (ICSR=0x%08X)\n",
           pxCurrentTCB->pcTaskName,
           simulated_icsr);
    xPortPendSVHandler();
}

#define taskYIELD() portYIELD()

static void led_step(void *arg)
{
    (void)arg;
    printf("[%s] toggle LED, run=%u\n", pxCurrentTCB->pcTaskName, ++pxCurrentTCB->runs);
}

static void uart_step(void *arg)
{
    (void)arg;
    printf("[%s] poll UART, run=%u\n", pxCurrentTCB->pcTaskName, ++pxCurrentTCB->runs);
}

int main(void)
{
    TCB_t led;
    TCB_t uart;

    list_init(&ready_list);
    tcb_init(&led, led_step, "LED");
    tcb_init(&uart, uart_step, "UART");
    list_insert_tail(&ready_list, &led.xStateListItem);
    list_insert_tail(&ready_list, &uart.xStateListItem);
    pxCurrentTCB = &led;

    printf("Ch6 v4: taskYIELD and PendSV semantics\n\n");
    for (int i = 0; i < 6; ++i) {
        pxCurrentTCB->step(pxCurrentTCB->parameter);
        taskYIELD();
    }

    printf("\nThis PC demo simulates the control flow; Cortex-M saves real registers in PendSV.\n");
    return 0;
}
