#ifndef TASK_H
#define TASK_H
#include <stdint.h>
#include <stdbool.h>
#include "container_of.h"
typedef struct ListNode { struct ListNode *next; struct ListNode *prev; } ListNode_t;
typedef struct { ListNode_t node; uint32_t *sp; uint8_t priority; uint8_t state; char name[16]; } TCB_t;
#define TCB_FROM_NODE(ptr) container_of(ptr, TCB_t, node)
void list_init(ListNode_t *head);
void list_insert_tail(ListNode_t *head, ListNode_t *node);
int list_is_empty(const ListNode_t *head);
ListNode_t *list_get_first(const ListNode_t *head);
void list_remove(ListNode_t *node);
void task_init(TCB_t *task, uint8_t priority, const char *name);
void task_print(const TCB_t *task);
#endif