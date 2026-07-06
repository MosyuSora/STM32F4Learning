# Chapter6 code

本目录按 Chapter6 的手撕路线组织。每个版本只服务一个核心机制，避免把 FreeRTOS 写成一坨“迷你全家桶”。能在 PC 上编译运行的 demo 使用标准 C；启动第一个任务和 PendSV 这类硬件相关主题使用明确标注的教学模型。

当前环境如果没有 C 编译器，运行脚本会显示 `expected-output.txt`，不会伪装成本机已经编译执行。

| 版本 | 主题 | 对照源码 |
|------|------|----------|
| `v0_bare_loop` | 裸机主循环被慢工作拖住 | Chapter6 `## 1` |
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

运行单个版本：

```powershell
powershell -ExecutionPolicy Bypass -File .\v1_task_stack\run.ps1
```

代码验收标准：概念路径对得上 FreeRTOS 源码；能跑的 demo 要有输出，不能真实运行的模型要清楚标注边界。
