#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef uintptr_t StackType_t;
typedef unsigned UBaseType_t;
typedef void (*TaskFunction_t)(void *);

#define STACK_WORDS 64u
#define INITIAL_XPSR ((StackType_t)0x01000000u)
#define TASK_RETURN_ADDRESS ((StackType_t)0xDEADBEEFu)
#define INITIAL_EXC_RETURN ((StackType_t)0xFFFFFFFDu)

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *pvOwner;
} ListNode_t;

typedef struct TCB {
    StackType_t *pxTopOfStack;
    ListNode_t state_node;
    StackType_t *pxStack;
    size_t uxStackDepth;
    TaskFunction_t pxTaskCode;
    void *pvParameters;
    UBaseType_t uxPriority;
    const char *pcTaskName;
} TCB_t;

typedef TCB_t *TaskHandle_t;

static ListNode_t ready_list;

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

static StackType_t *initialise_stack(StackType_t *stack,
                                     size_t depth,
                                     TaskFunction_t entry,
                                     void *parameter)
{
    StackType_t *top = stack + depth;
    *--top = INITIAL_XPSR;
    *--top = (StackType_t)entry;
    *--top = TASK_RETURN_ADDRESS;
    *--top = 0x12121212u;
    *--top = 0x03030303u;
    *--top = 0x02020202u;
    *--top = 0x01010101u;
    *--top = (StackType_t)parameter;
    *--top = INITIAL_EXC_RETURN;
    for (int reg = 11; reg >= 4; --reg) {
        *--top = (StackType_t)(0x44440000u + (unsigned)reg);
    }
    return top;
}

static TaskHandle_t xTaskCreateStatic(TaskFunction_t entry,
                                      const char *name,
                                      size_t stack_depth,
                                      void *parameter,
                                      UBaseType_t priority,
                                      StackType_t *stack,
                                      TCB_t *tcb)
{
    tcb->pxStack = stack;
    tcb->uxStackDepth = stack_depth;
    tcb->pxTaskCode = entry;
    tcb->pvParameters = parameter;
    tcb->uxPriority = priority;
    tcb->pcTaskName = name;
    tcb->pxTopOfStack = initialise_stack(stack, stack_depth, entry, parameter);
    tcb->state_node.next = NULL;
    tcb->state_node.prev = NULL;
    tcb->state_node.pvOwner = tcb;
    list_insert_tail(&ready_list, &tcb->state_node);
    return tcb;
}

static void led_task(void *arg) { (void)arg; }
static void uart_task(void *arg) { (void)arg; }

int main(void)
{
    int led_parameter = 0x1111;
    int uart_parameter = 0x2222;
    TCB_t led_tcb;
    TCB_t uart_tcb;
    StackType_t led_stack[STACK_WORDS] = {0};
    StackType_t uart_stack[STACK_WORDS] = {0};

    list_init(&ready_list);

    TaskHandle_t led = xTaskCreateStatic(led_task, "LED", STACK_WORDS,
                                         &led_parameter, 1, led_stack, &led_tcb);
    TaskHandle_t uart = xTaskCreateStatic(uart_task, "UART", STACK_WORDS,
                                          &uart_parameter, 2, uart_stack, &uart_tcb);

    printf("Ch6 v3: static task creation\n\n");
    printf("  task=%s handle=%p stack=%p top=%p priority=%u\n",
           led->pcTaskName, (void *)led, (void *)led->pxStack,
           (void *)led->pxTopOfStack, led->uxPriority);
    printf("  task=%s handle=%p stack=%p top=%p priority=%u\n",
           uart->pcTaskName, (void *)uart, (void *)uart->pxStack,
           (void *)uart->pxTopOfStack, uart->uxPriority);
    printf("\n  LED state_node owner=%p\n", led->state_node.pvOwner);
    printf("  UART state_node owner=%p\n", uart->state_node.pvOwner);
    printf("\nUser owns memory; kernel only initialises and links it.\n");
    return 0;
}
