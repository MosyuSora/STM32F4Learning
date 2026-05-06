#include <stdio.h>
#include "task.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v9: container_of Macro\n\n");
    TCB_t task_a, task_b, task_c;
    ListNode_t ready_list;
    task_init(&task_a,1,"TaskA"); task_init(&task_b,2,"TaskB"); task_init(&task_c,3,"TaskC");
    list_init(&ready_list);
    list_insert_tail(&ready_list,&task_a.node);
    list_insert_tail(&ready_list,&task_b.node);
    list_insert_tail(&ready_list,&task_c.node);
    printf("=== Iterating: node ptr -> TCB ===\n");
    ListNode_t *iter; TCB_t *tcb;
    for(iter=ready_list.next;iter!=&ready_list;iter=iter->next){
        printf("  ListNode at %p -> ",(void*)iter);
        tcb=TCB_FROM_NODE(iter);
        printf("TCB %s at %p\n",tcb->name,(void*)tcb);
    }
    printf("\n  offsetof(TCB_t,node)=%zu (compile-time, zero-cost)\n",offsetof(TCB_t,node));
    printf("\nDone.\n");
    return 0;
}