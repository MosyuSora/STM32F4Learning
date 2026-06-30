# Chapter6 code

本目录按 Chapter6 的手撕路线组织。当前先建立版本骨架，后续每个版本只服务一个核心机制，避免把 FreeRTOS 写成一坨“迷你全家桶”。

| 版本 | 主题 | 对照源码 |
|------|------|----------|
| `v1_task_stack` | 任务入口和独立栈 | `port.c:pxPortInitialiseStack()` |
| `v2_tcb` | 最小 TCB | `tasks.c:TCB_t` |
| `v3_kernel_list` | 内核链表 | `list.c`, `include/list.h` |
| `v4_static_task_create` | 静态任务创建 | `tasks.c:xTaskCreateStatic()` |
| `v5_priority_scheduler` | 优先级调度和 round robin | `tasks.c:vTaskSwitchContext()` |
| `v6_start_first_task` | 启动第一个任务 | `port.c:vPortSVCHandler()` |
| `v7_pendsv_switch` | PendSV 上下文切换 | `port.c:xPortPendSVHandler()` |
| `v8_delay_blocked_list` | delay 和 tick 唤醒 | `tasks.c:vTaskDelay()`, `xTaskIncrementTick()` |
| `v9_queue` | 阻塞队列 | `queue.c` |
| `v10_mutex_inheritance` | mutex 和优先级继承 | `queue.c` mutex 路径 |
| `v11_heap4_allocator` | 简化 heap_4 allocator | `portable/MemMang/heap_4.c` |

代码验收标准：概念路径对得上 FreeRTOS 源码；能跑不是必要条件。
