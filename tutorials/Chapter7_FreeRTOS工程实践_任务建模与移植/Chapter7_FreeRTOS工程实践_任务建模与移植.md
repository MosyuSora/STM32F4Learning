# Chapter 7 FreeRTOS 工程实践：任务建模与移植

Chapter6 解决“FreeRTOS 核心机制为什么这样写”。本章解决另一个问题：

> 真正做 STM32 FreeRTOS 工程时，任务怎么拆，通信怎么选，优先级怎么定，系统怎么证明跑得过来，移植层又如何接上平台。

本章不再重复手撕内核，而是把 Chapter6 的源码模型翻译成工程决策。

## 0 本章阅读路线

```text
从手撕内核到真实工程
  -> 任务划分
  -> 任务数学模型
  -> 甘特图
  -> 可调度性分析
  -> 任务通信设计
  -> 优先级反转工程处理
  -> ISR 与 RTOS API 边界
  -> FreeRTOSConfig.h
  -> 移植层
  -> CMSIS-RTOS / CubeMX
  -> STM32 工程接入
  -> 裁切优化
  -> 工程案例复盘
```

配套材料：

- [`materials/README.md`](materials/README.md)：任务表、甘特图、可调度性分析和 CubeMX/CMSIS 对照材料规划。
- [`code/README.md`](code/README.md)：后续工程示例规划。

## 1 从手撕内核到真实工程

### 1.1 Chapter6 的最小内核缺少什么

Chapter6 的教学内核强调机制，不强调工程完备性。真实工程还需要配置、观测、错误处理、移植层、任务通信设计和资源预算。

### 1.2 工程实践关注的是可维护、可分析、可移植

FreeRTOS 工程不是“API 调通就结束”。任务之间的依赖、优先级、阻塞时间、栈大小和中断边界都会影响系统可靠性。

### 1.3 FreeRTOS 工程的三条主线：任务设计、任务通信、平台移植

本章围绕三条线展开：任务设计、任务通信、平台移植。

## 2 任务划分方法

### 2.1 一个功能什么时候应该拆成任务

拆任务的依据是独立时序需求、阻塞点、优先级需求和生命周期，而不是代码看起来很多。

### 2.2 任务不是线程数量竞赛

任务越多，调度和通信成本越高。工程上要避免把每个小函数都拆成任务。

### 2.3 周期任务、事件任务、后台任务

三类任务的调度特性不同，建模方式也不同。

### 2.4 CPU 密集型任务和 I/O 等待型任务

CPU 密集型任务关注执行时间；I/O 等待型任务关注阻塞点和唤醒路径。

### 2.5 STM32 工程里的典型任务划分

例子：采样任务、通信任务、控制任务、日志任务、UI 任务、后台诊断任务。

## 3 任务的数学模型

### 3.1 周期 `T`

任务多久释放一次执行需求。

### 3.2 最坏执行时间 `C / WCET`

任务一次运行最多占用多少 CPU 时间。

### 3.3 截止时间 `D`

任务必须在多久内完成。

### 3.4 优先级 `P`

FreeRTOS 的固定优先级调度需要把工程重要性和时间约束映射到优先级。

### 3.5 阻塞时间 `B`

任务可能因为 mutex、queue、临界区或中断屏蔽而等待。

### 3.6 抖动 jitter

任务实际开始运行时间相对理论释放时间的偏差。

### 3.7 用模型描述一个 FreeRTOS 任务集

给出任务表模板：任务名、类型、周期、WCET、deadline、优先级、栈大小、通信对象、阻塞来源。

## 4 甘特图和调度直觉

### 4.1 为什么要画甘特图

甘特图把调度器行为可视化，帮助发现响应延迟、饥饿、阻塞和优先级反转。

### 4.2 绝对优先级调度的时间线

展示高优先级任务如何抢占低优先级任务。

### 4.3 同优先级 round robin 的时间线

展示同优先级任务如何共享时间片。

### 4.4 delay、queue wait、semaphore wait 在图上怎么表示

把 Chapter6 的 ready/blocked 状态映射到工程时间线。

### 4.5 从甘特图发现饥饿、阻塞和响应延迟

用图反推任务拆分和优先级是否合理。

## 5 可调度性分析

### 5.1 CPU 利用率不是唯一指标

CPU 利用率低不代表一定满足 deadline，阻塞和优先级配置同样重要。

### 5.2 Rate Monotonic 的基本直觉

周期越短的任务通常优先级越高，但工程里还要考虑阻塞和外设时序。

### 5.3 Response Time Analysis 的工程化用法

用响应时间估算判断任务是否能在 deadline 前完成。

### 5.4 阻塞时间如何影响高优先级任务

mutex、临界区和中断屏蔽会直接进入高优先级任务的最坏响应时间。

### 5.5 FreeRTOS 项目里如何估算 WCET

方法：GPIO 翻转测量、DWT cycle counter、trace 工具、保守估计。

### 5.6 分析结果如何反推任务优先级

分析用来调整优先级、拆任务、缩短临界区和修改通信方式。

## 6 任务通信设计

### 6.1 通信不是 API 选择题，而是依赖关系设计

先画数据流和等待关系，再选 FreeRTOS API。

### 6.2 queue：数据流

适合传递结构化数据和生产者/消费者模型。

### 6.3 semaphore：事件通知和资源计数

适合无数据事件或资源计数。

### 6.4 mutex：共享资源互斥

适合保护共享外设、共享缓冲区和非重入库。

### 6.5 direct-to-task notification：轻量事件

适合一对一事件通知，常用于 ISR 唤醒任务。

### 6.6 event group：多条件等待

适合多个 bit 条件组合。

### 6.7 stream/message buffer：字节流和消息流

适合串口、协议流和可变长度消息。

### 6.8 如何避免把系统通信写成一团网

原则：单向数据流、明确 owner、少共享、多消息、少跨层调用。

## 7 优先级反转的工程处理

### 7.1 再看优先级反转：源码机制到工程后果

Chapter6 讲源码机制，本节讲它如何拖垮真实响应时间。

### 7.2 mutex priority inheritance 什么时候有效

只对 mutex 有意义，且只能缓解持锁导致的优先级反转。

### 7.3 binary semaphore 为什么不提供同样语义

binary semaphore 没有所有权，不适合表达互斥资源所有者。

### 7.4 缩短临界区

持锁时间越短，阻塞时间上界越小。

### 7.5 避免低优先级任务长期持锁

低优先级任务持锁时不要做 delay、IO 等长时间动作。

### 7.6 用甘特图分析一次优先级反转

用时间线展示低、中、高三个任务的优先级反转和继承过程。

## 8 中断与 FreeRTOS API 边界

### 8.1 ISR 里为什么不能随便调用普通 API

普通 API 可能阻塞，ISR 不能阻塞。

### 8.2 `FromISR` API 的命名规律

ISR 中使用 `xxxFromISR()` 族 API。

### 8.3 `xHigherPriorityTaskWoken`

它把“是否唤醒了更高优先级任务”传出 ISR。

### 8.4 中断唤醒任务后的 PendSV

ISR 尾部触发 PendSV，让任务切换推迟到安全位置发生。

### 8.5 Cortex-M 中断优先级和 `configMAX_SYSCALL_INTERRUPT_PRIORITY`

只有优先级不高于该阈值的中断才能调用 FreeRTOS API。

### 8.6 常见错误：中断优先级配置错导致系统异常

CubeMX/HAL 工程里容易混淆抢占优先级、子优先级和 FreeRTOS syscall priority。

## 9 `FreeRTOSConfig.h`：裁切和配置

### 9.1 `configUSE_PREEMPTION`

是否启用抢占式调度。

### 9.2 `configUSE_TIME_SLICING`

同优先级任务是否轮转。

### 9.3 `configTICK_RATE_HZ`

tick 频率影响延时精度、调度开销和功耗。

### 9.4 `configMAX_PRIORITIES`

优先级数量不是越多越好。

### 9.5 `configMINIMAL_STACK_SIZE`

最小栈大小只是 idle task 的参考，不是所有任务的通用答案。

### 9.6 静态/动态内存配置

`configSUPPORT_STATIC_ALLOCATION` 和 `configSUPPORT_DYNAMIC_ALLOCATION` 影响任务、队列等对象创建方式。

### 9.7 hook、assert、trace 配置

工程调试时应打开 assert 和必要 hook，发布时按资源预算裁切。

### 9.8 如何根据项目裁切功能

从任务通信方式、内存策略、调试需求和功耗需求反推配置。

## 10 FreeRTOS 移植层

### 10.1 FreeRTOS 为什么要分 kernel 和 portable

kernel 负责通用 RTOS 逻辑，portable 负责架构和编译器相关细节。

### 10.2 `portable.h`、`portmacro.h`、`port.c`

三者共同定义类型、临界区、yield、tick、上下文切换和调度器启动。

### 10.3 平台需要提供哪些东西

平台需要提供启动文件、中断向量表、SysTick 或替代 tick、临界区屏蔽方式和内存布局。

### 10.4 Cortex-M 移植层如何对接 SVC、PendSV、SysTick

把 Chapter6 的 SVC/PendSV/SysTick 机制放回真实工程。

### 10.5 编译器、架构、启动文件和中断向量表

GCC/ARMCC/IAR 的端口文件不同，启动文件里 handler 名字也必须对上。

### 10.6 移植时最容易错的几个点

常见错误：handler 未映射、SysTick 冲突、中断优先级错误、栈/heap 配置不足。

## 11 CMSIS-RTOS 与 CubeMX 生成工程

### 11.1 CMSIS 是什么：ARM 提供的抽象层

CMSIS 提供 Cortex-M 内核、设备头文件和 RTOS 抽象接口。

### 11.2 CMSIS-RTOS v1/v2 和 FreeRTOS 的关系

CMSIS-RTOS 是 API 包装层，底层可以是 FreeRTOS。

### 11.3 CubeMX 生成的 FreeRTOS 工程包含什么

包含内核源码、CMSIS-RTOS 适配层、`FreeRTOSConfig.h`、任务初始化代码和 HAL 集成。

### 11.4 `osThreadNew()` 如何落到 FreeRTOS 任务

CMSIS API 最终会包装到 FreeRTOS 的任务创建路径。

### 11.5 CubeMX 工程和我们手写工程的区别

CubeMX 多了一层配置生成和 CMSIS 适配；手写工程更直接，但需要自己保证所有移植细节正确。

### 11.6 什么时候用 CMSIS API，什么时候直接用 FreeRTOS API

若强调跨 RTOS 抽象可用 CMSIS；若强调 FreeRTOS 特性和源码对应，直接用 FreeRTOS API 更清楚。

## 12 STM32 工程接入 FreeRTOS

### 12.1 从裸机工程迁移到 FreeRTOS 工程

先保留底层驱动，再把业务循环拆入任务。

### 12.2 SysTick 归 HAL 还是归 FreeRTOS

HAL 和 FreeRTOS 都可能使用 tick，需要明确时间基准归属。

### 12.3 HAL_Delay 和 RTOS delay 的关系

调度器启动前后 delay 语义不同，任务里优先使用 RTOS delay。

### 12.4 外设驱动放在任务里还是中断里

中断负责快速响应和唤醒任务，任务负责复杂处理。

### 12.5 DMA + 中断 + 任务通知的典型组合

DMA 完成中断唤醒处理任务，是 STM32 FreeRTOS 工程里的常见通信链路。

### 12.6 一个最小多任务 STM32 工程结构

建议结构：`app/`、`bsp/`、`drivers/`、`platform/`、`freertos/`。

## 13 移植后的裁切与优化

### 13.1 减少不需要的 API

通过配置宏关闭不用的功能，降低代码体积。

### 13.2 栈大小估算与 high water mark

用 `uxTaskGetStackHighWaterMark()` 观察栈余量。

### 13.3 heap 大小估算与碎片控制

结合 Chapter6 的 heap 模型分析动态分配风险。

### 13.4 tick 频率对实时性和开销的影响

tick 越高，延时粒度越细，但调度开销越大。

### 13.5 临界区长度和中断延迟

临界区会影响中断响应和高优先级任务唤醒延迟。

### 13.6 trace 工具和运行时观测

trace、GPIO 翻转、DWT cycle counter 都是工程验证手段。

## 14 工程案例复盘

### 14.1 构造一个典型 STM32 FreeRTOS 小系统

案例包含采样、控制、通信、日志和诊断任务。

### 14.2 任务表：周期、优先级、栈大小、通信方式

用任务表把工程设计落到可检查数据。

### 14.3 甘特图分析

画出关键时间窗口内的任务运行顺序。

### 14.4 可调度性检查

检查 deadline、阻塞时间和 CPU 利用率。

### 14.5 通信链路检查

检查队列长度、唤醒路径和共享资源 owner。

### 14.6 移植层和 CubeMX 配置检查

检查 handler、tick、heap、priority 和 CMSIS 适配层。

### 14.7 从源码机制回到工程决策

本章最后把 Chapter6 的机制模型映射回工程判断：每一个配置和 API 选择背后，都对应一个内核行为。
