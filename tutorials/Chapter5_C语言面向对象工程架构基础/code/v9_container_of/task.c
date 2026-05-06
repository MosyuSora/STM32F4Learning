#include "task.h"
#include <stdio.h>
#include <string.h>
void list_init(ListNode_t *head){head->next=head;head->prev=head;}
void list_insert_tail(ListNode_t *head, ListNode_t *node){node->next=head;node->prev=head->prev;head->prev->next=node;head->prev=node;}
int list_is_empty(const ListNode_t *head){return head->next==head;}
ListNode_t *list_get_first(const ListNode_t *head){if(list_is_empty(head))return NULL;return head->next;}
void list_remove(ListNode_t *node){node->prev->next=node->next;node->next->prev=node->prev;node->next=NULL;node->prev=NULL;}
void task_init(TCB_t *task, uint8_t priority, const char *name){task->sp=NULL;task->priority=priority;task->state=0;strncpy(task->name,name,sizeof(task->name)-1);printf("  [Task] %s created, priority=%d\n",name,priority);}
void task_print(const TCB_t *task){printf("  Task %s: priority=%d addr=%p\n",task->name,task->priority,(void*)task);}