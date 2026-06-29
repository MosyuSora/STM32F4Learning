#include <stdint.h>
#include <stdio.h>

#define configMAX_PRIORITIES 3u
#define configUSE_PREEMPTION 1u
#define configUSE_TIME_SLICING 1u
#define pdFALSE 0
#define pdTRUE 1
#define portNVIC_PENDSVSET_BIT (1u << 28)

typedef uintptr_t StackType_t;
typedef unsigned UBaseType_t;
typedef int BaseType_t;
typedef void (*TaskStep_t)(void *);

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *pvOwner;
} ListNode_t;

typedef struct {
    UBaseType_t uxNumberOfItems;
    ListNode_t *pxIndex;
    ListNode_t xListEnd;
} List_t;

typedef struct TCB {
    volatile StackType_t *pxTopOfStack;
    ListNode_t xStateListItem;
    UBaseType_t uxPriority;
    const char *pcTaskName;
    TaskStep_t step;
    void *pvParameters;
    unsigned runs;
} TCB_t;

typedef TCB_t *TaskHandle_t;

static List_t pxReadyTasksLists[configMAX_PRIORITIES];
static TCB_t * volatile pxCurrentTCB;
static unsigned xTickCount;
static uint32_t simulated_icsr;

static void vListInitialise(List_t *list)
{
    list->uxNumberOfItems = 0u;
    list->pxIndex = &list->xListEnd;
    list->xListEnd.next = &list->xListEnd;
    list->xListEnd.prev = &list->xListEnd;
    list->xListEnd.pvOwner = NULL;
}

static int listLIST_IS_EMPTY(const List_t *list)
{
    return list->uxNumberOfItems == 0u;
}

static unsigned listCURRENT_LIST_LENGTH(const List_t *list)
{
    return list->uxNumberOfItems;
}

static void vListInsertEnd(List_t *list, ListNode_t *node)
{
    ListNode_t *index = list->pxIndex;

    node->next = index->next;
    node->prev = index;
    index->next->prev = node;
    index->next = node;
    ++list->uxNumberOfItems;
}

static TCB_t *listGET_OWNER_OF_NEXT_ENTRY(List_t *list)
{
    list->pxIndex = list->pxIndex->next;
    if (list->pxIndex == &list->xListEnd) {
        list->pxIndex = list->pxIndex->next;
    }

    return (TCB_t *)list->pxIndex->pvOwner;
}

static void prvInitialiseTaskLists(void)
{
    for (UBaseType_t i = 0; i < configMAX_PRIORITIES; ++i) {
        vListInitialise(&pxReadyTasksLists[i]);
    }
}

static TaskHandle_t xTaskCreateStatic(TaskStep_t step,
                                      const char *name,
                                      UBaseType_t priority,
                                      TCB_t *tcb)
{
    if (priority >= configMAX_PRIORITIES) {
        priority = configMAX_PRIORITIES - 1u;
    }

    tcb->pxTopOfStack = NULL;
    tcb->uxPriority = priority;
    tcb->pcTaskName = name;
    tcb->step = step;
    tcb->pvParameters = NULL;
    tcb->runs = 0;
    tcb->xStateListItem.next = NULL;
    tcb->xStateListItem.prev = NULL;
    tcb->xStateListItem.pvOwner = tcb;
    vListInsertEnd(&pxReadyTasksLists[priority], &tcb->xStateListItem);
    return tcb;
}

static UBaseType_t prvHighestReadyPriority(void)
{
    for (int p = (int)configMAX_PRIORITIES - 1; p >= 0; --p) {
        if (!listLIST_IS_EMPTY(&pxReadyTasksLists[p])) {
            return (UBaseType_t)p;
        }
    }
    return 0;
}

static void prvSelectNextTask(void)
{
    UBaseType_t priority = prvHighestReadyPriority();
    pxCurrentTCB = listGET_OWNER_OF_NEXT_ENTRY(&pxReadyTasksLists[priority]);
}

static void xPortPendSVHandler(void)
{
    const char *old_name = pxCurrentTCB ? pxCurrentTCB->pcTaskName : "none";
    prvSelectNextTask();
    printf("  [PendSV] %s -> %s\n", old_name, pxCurrentTCB->pcTaskName);
}

static void portYIELD(void)
{
    simulated_icsr = portNVIC_PENDSVSET_BIT;
    printf("  [portYIELD] set PENDSVSET (ICSR=0x%08X)\n", simulated_icsr);
    xPortPendSVHandler();
}

static BaseType_t xTaskIncrementTick(void)
{
    ++xTickCount;
    printf("  [Tick] xTickCount=%u\n", xTickCount);

#if configUSE_PREEMPTION && configUSE_TIME_SLICING
    if (pxCurrentTCB != NULL &&
        listCURRENT_LIST_LENGTH(&pxReadyTasksLists[pxCurrentTCB->uxPriority]) > 1u) {
        return pdTRUE;
    }
#endif

    return pdFALSE;
}

static void xPortSysTickHandler(void)
{
    printf("  [SysTick] enter\n");
    if (xTaskIncrementTick() != pdFALSE) {
        printf("  [SysTick] time slicing asks for PendSV\n");
        portYIELD();
    }
}

static void led_step(void *arg)
{
    (void)arg;
    printf("[%s] run=%u\n", pxCurrentTCB->pcTaskName, ++pxCurrentTCB->runs);
}

static void uart_step(void *arg)
{
    (void)arg;
    printf("[%s] run=%u\n", pxCurrentTCB->pcTaskName, ++pxCurrentTCB->runs);
}

int main(void)
{
    TCB_t led_tcb;
    TCB_t uart_tcb;

    prvInitialiseTaskLists();
    xTaskCreateStatic(led_step, "LED", 1, &led_tcb);
    xTaskCreateStatic(uart_step, "UART", 1, &uart_tcb);
    prvSelectNextTask();

    printf("Ch6 v5: FreeRTOS-shaped tiny scheduler\n\n");
    printf("Name mapping: pxCurrentTCB, xTaskCreateStatic, pxReadyTasksLists, portYIELD, xTaskIncrementTick\n\n");

    for (int i = 0; i < 6; ++i) {
        pxCurrentTCB->step(pxCurrentTCB->pvParameters);
        xPortSysTickHandler();
    }

    printf("\nCompare with FreeRTOS: tasks.c + list.c + portable/GCC/ARM_CM4F/port.c\n");
    return 0;
}
