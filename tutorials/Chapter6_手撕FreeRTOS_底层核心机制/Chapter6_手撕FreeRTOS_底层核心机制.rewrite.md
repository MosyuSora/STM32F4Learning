
# Chapter 6 手撕 FreeRTOS：从一个人干活，到组建一个班组

> 本文件是第六章的可读性重写稿，逐节写入、逐节验收，定稿后替换正式章节。
> 主线比喻：四个节奏迥异的"工人"（LED / SENSOR / COMM / LOG）共用一张工作台（CPU）。
> 裸机是让他们排一队轮流上台，FreeRTOS 就是那个把工作台调度起来的班组长。

## 0 开场：四个工人挤在一张工作台上

先把班组里的四个人请到台前。LED 负责让人一眼看出"系统还活着"，每隔 50 ms 稳稳翻转一下；SENSOR 负责周期采样，每 20 ms 取一个新数据，采晚了数据就"馊"了；COMM 负责跟外部设备打交道，平时闲着，一旦来了命令就得赶紧回话；LOG 负责把运行信息从串口慢慢吐出去，它天生就慢，还不着急。

四个人单拎出来都不难，难在他们的节奏完全不是一回事：LED 要**稳**，SENSOR 要**准**，COMM 要**快**，LOG 天生**慢**。

| 工人 | 节奏 | 它最在乎的证据 | 将来对应的 FreeRTOS 机制 |
| --- | --- | --- | --- |
| LED | 固定周期、动作轻 | 翻转时刻稳不稳 | Delay、就绪列表、调度 |
| SENSOR | 固定周期、要数据新鲜 | 采样点漂没漂 | vTaskDelayUntil、任务栈、队列 |
| COMM | 外部事件驱动、响应压力大 | 从收到事件到回话的耗时 | 优先级、队列、PendSV |
| LOG | 慢 I/O、后台处理 | 队列积压、单次输出耗时 | 队列、互斥锁、低优先级任务 |

裸机的做法，是让这四个人**排一队、共用一张工作台**：轮到谁谁上，干完让位给下一个。队伍短的时候看不出毛病，可只要队伍里混进一个慢手，排在他后面的人就得干等——哪怕后面那位的活又急又短。

这不是假想。把四个人塞进一个 `while(1)`，让 LOG 偶尔"卡壳"一下，问题立刻就出来了。看 [`v0_bare_loop`](code/v0_bare_loop/demo.c)，它只保留一个现象：LOG 一旦忙起来，LED 的心跳和 SENSOR 的采样时刻会跟着一起被推后。

```c
static void led_heartbeat(void) {
    if (now_ms >= next_led_ms) {
        printf("t=%03d LED toggle\n", now_ms);
        next_led_ms += 50;
    }
}

static int log_flush(void) {
    if (now_ms == 40) log_burst_left = 3;      /* t=40 起 LOG 开始忙 */
    if (log_burst_left > 0) {
        printf("t=%03d LOG flush chunk, main loop blocked 35ms\n", now_ms);
        log_burst_left--;
        return 35;                             /* 这一轮主循环被占住 35ms */
    }
    return 5;
}

int main(void) {
    while (now_ms <= 140) {
        led_heartbeat();
        sensor_sample();
        now_ms += log_flush();                 /* 时间被 log_flush 改写 */
    }
}
```

代码先别逐行抠，盯住时间是怎么被 `log_flush()` 改写的：

```output
t=000 LED toggle
t=000 SENSOR sample
t=020 SENSOR sample
t=040 SENSOR sample
t=040 LOG flush chunk, main loop blocked 35ms
t=075 LED toggle
t=075 SENSOR sample
t=075 LOG flush chunk, main loop blocked 35ms
t=110 LED toggle
```

从 `t=040` 开始，LOG 进入忙碌期，每卡一次，主循环的时间就往后跳 35 ms。于是 LED 本该在 `t=050` 附近的下一次翻转，被硬生生推到了 `t=075`；SENSOR 也不再稳稳地每 20 ms 出现一次。注意：**LED 和 SENSOR 自己一点没变慢，是被同一条执行线拖住了。** 这条被慢手拖长的时间线，画出来是这样：

![裸机时间线被慢日志拉长](img/fig-009.png)

看这张图，先找 LOG 那一格，再看它后面被撑开的空白，最后看 LED、SENSOR 原本的节奏怎样跟着整体偏移。所有后面要讲的东西——任务、Delay、队列、优先级——都不是凭空发明的名词，而是为了修好这一条线：**让慢手不再把所有人拴在一起。**

要修它，得先换一个当"班组长"的思路。班组长不会把四个人真的变成四台机器同时开工——单核 MCU 同一时刻仍然只有一个人在台上。他做的是另一件事：允许每个人在"该等"的时候主动退到一边（LED 等时间、COMM 等事件、LOG 等串口），并且记住谁在等、谁能上、谁到点了该回来。于是"谁在拖谁"这件原本藏在时间线里的糊涂账，变成了可以看见、可以解释、可以调试的系统状态。

这一章就沿着这条线走：一个工作，怎样从一个普通函数，变成内核能暂停、唤醒、调度、切换、协作、还要算清内存账的**任务**。`TCB`、`就绪列表`、`PendSV`、`队列`、`heap_4` 这些词会陆续登场，但它们始终服务于同一批很具体的问题——"这个工人现在为什么没上台""到点了为什么还没轮到他""内存看着够用，为什么建对象却失败了"。

读法上给一个建议：第一遍从这里顺着往下读到"全景"那一节，只抓"现象→机制→demo 证据"的主干，源码链接看到名字就行；等主干立住了，再带着具体问题回头去对账 `tasks.c`、`list.c`、`queue.c`、`port.c`、`heap_4.c`。四个工人会一路跟到底，机制再多，也总能落回他们中某一个人的某一次卡壳。

## 1 任务：一段能暂停、还能回来接着干的活

裸机的死结，在于所有活共用一条执行线。要拆开它，得先换一种眼光看"一件活"。

普通函数是**一次性**的：调用者进来，函数从头跑到尾，返回，控制权还给调用者，这一轮的局部变量、返回地址随调用链一起烟消云散。LED 若只是个普通函数，它每次都得从入口重来，上一轮做到哪儿、算到几，全不记得。

任务不是这样。LED 任务一旦进了自己的循环，就**长期活着**：翻转一下 LED，调用 Delay 主动让出工作台；等时间到、班组长再选中它，它不是从整个程序开头重启，而是**从让出的那个点后面接着走**。这个"能从原地接着走"，正是任务栈和上下文切换要解决的事。

一句话先钉住：**任务是一段能被暂停、保存、恢复的执行流。** 后面所有字段和源码，都绕着这句话转。

普通函数的现场跟着调用链走，任务的现场跟着任务对象走——这个差别，决定了为什么要额外准备任务栈、TCB 和 PendSV。

![普通函数与 FreeRTOS 任务的对比](img/fig-002.png)

图左边，普通函数跑完就回到调用者，现场随调用链消失；右边，任务在让出点附近把现场存进自己的栈，之后再从那里恢复。看起来只是多分了几块内存，工程含义却很大：**现场不再只有一份了。** 每个工人都有自己的一份，班组长把工作台交给谁的现场，台上就"变成"谁在干活。

### 1.1 每个任务先得有自己的一块现场

单核 CPU 同一刻只能干一件事，任务机制不是变出多个 CPU，而是给每个工作一块**自己的现场空间**：暂时不需要工作台就退下，需要继续时再被恢复。这样 LED 等时间的空档，SENSOR 可以采样；LOG 慢慢吐字的时候，COMM 不必陪它一起干耗。

这块现场，最小只要盯两个地址就够了：任务自己的**栈底**（stack base，这片现场归谁），和班组长将来要恢复的**栈顶**（top of stack，从哪儿把现场取回来）。[`v1_task_stack`](code/v1_task_stack/demo.c) 把这两个地址直接打出来，还顺手把"任务入口"和"参数"预先摆进了初始现场里：

```c
typedef struct {
    const char *name;
    TaskEntry entry;
    void *parameter;
    uint32_t stack[8];
    uint32_t *top_of_stack;
} MiniTaskStack;

static void initialise_stack(MiniTaskStack *task, const char *name,
                             TaskEntry entry, void *parameter) {
    task->name = name;
    task->entry = entry;
    task->parameter = parameter;
    task->top_of_stack = &task->stack[7];      /* 将来从这里恢复 */
    task->stack[7] = (uint32_t)(uintptr_t)entry;      /* 入口预先摆好 */
    task->stack[6] = (uint32_t)(uintptr_t)parameter;  /* 参数预先摆好 */
}
```

地址的具体数值每台机器都不一样，不用记；要看的是结构：

```output
init LED    stack_base=<addr> top=<addr> entry_slot=<entry> parameter_slot=<parameter>
init SENSOR stack_base=<addr> top=<addr> entry_slot=<entry> parameter_slot=<parameter>
scheduler restores LED stack
run task=LED parameter=heartbeat
scheduler restores SENSOR stack
run task=SENSOR period_ms=20
```

两个点：其一，LED 和 SENSOR 有各自不同的 `stack_base/top`，说明它们**不共用一份现场**；其二，"恢复 LED 栈"之后台上就是 LED，"恢复 SENSOR 栈"之后台上就是 SENSOR——入口和参数早就摆进了各自的初始现场，班组长恢复哪一份，谁就像"从那里开始运行"。

真实 Cortex-M 当然不会像 demo 这样直接 `led.entry()` 一调了事，它要靠寄存器、异常返回和栈帧来完成恢复。但 demo 把硬件细节先拿掉，只留下方向：**入口和参数摆进栈里，恢复对应现场，任务就像从那里活过来。**

### 1.2 任务栈里存的不只是局部变量

再往深一层问：如果 LED 在 Delay 之前有个局部计数值，恢复之后这个值居然还在，它到底藏哪儿了？答案不是"内核帮你记住了每个变量"，而是每个任务都有自己的栈——局部变量、函数调用链在这里，上下文切换时 CPU 要恢复的那组寄存器现场也压在这里。

所以任务栈是**双重身份**：既是 C 语言调用链的空间，也是内核恢复执行流的依据。任务被切走时，关键寄存器和返回现场被压进这片栈；被恢复时，CPU 再从这里把现场取回。也正因如此，任务里一个较大的临时数组、一段深调用、一次 `printf` 格式化，都会实实在在地吃掉栈水位——它们和"现场保存"共用同一片空间。

这也解释了写任务时那条最容易搞反的直觉：任务函数**不是**会被反复从头调用的普通函数。它进循环后长期存在，Delay 或阻塞只是让出工作台，从不把函数"重启"。把这一点想通，才会理解为什么栈水位要在**高峰路径**上盯——通信高峰、日志高峰、错误处理里，往往正是调用链最深、临时缓冲最大的时候。

### 1.3 回到源码：第一次现场是谁摆好的

demo 说明了任务要有自己的现场，回到 FreeRTOS 只追三个问题：第一次的现场在哪里摆好、谁去摆、摆好的栈顶交给谁记住。

把端口层的 `pxPortInitialiseStack()`（[port.c:202](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:202)）压成伪代码，它其实就是 demo 那几行的"硬件完整版"：

```c
top--;  *top = xPSR;             /* 213: 初始状态字 */
top--;  *top = task_entry;       /* 215: PC —— 任务第一条指令 */
top--;  *top = task_return_trap; /* 217: LR —— 任务不允许 return */
top -= 5;
*top = pvParameters;             /* 221: R0 —— 任务入口的第一个参数 */
top--;  *top = exc_return;
top -= 8;                        /* R4-R11 预留 */
return top;                      /* 230: 新栈顶，交给 TCB */
```

demo 输出里的三个词，逐一对得上真实源码：`entry_slot` 就是那个 PC，`parameter_slot` 就是 R0，`top` 就是最后返回、将来要被 TCB 记住的新栈顶。真实版本多处理了 xPSR、LR、异常返回和寄存器保存区，但主线证据始终是三个：**入口、参数、栈顶。**

谁来调用它摆现场？是创建任务时的 `prvInitialiseNewTask()`（[tasks.c:1816](reference/rtos_src/FreeRTOS-Kernel/tasks.c:1816)）：它先算好栈顶地址，再把入口、参数、栈顶交给端口层布置初始栈帧。摆好的栈顶最终落进 TCB 的第一个字段 `pxTopOfStack`（[tasks.c:377](reference/rtos_src/FreeRTOS-Kernel/tasks.c:377)）——它被特意放在结构体最前面，因为上下文切换的汇编要用最快的方式够到它。

| demo 里的动作 | FreeRTOS 源码里的证据 | 要理解的含义 |
| --- | --- | --- |
| `entry_slot = entry` | [port.c:215](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:215) 把入口放进初始 PC 位 | 第一次恢复现场，CPU 就落到任务入口 |
| `parameter_slot = parameter` | [port.c:221](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:221) 把参数放到 R0 位 | 任务入口第一次运行就能拿到 `pvParameters` |
| `return top_of_stack` | [port.c:230](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:230) 返回新栈顶 | 栈顶交给 `pxTopOfStack`，将来靠它恢复 |

![任务初始栈帧：函数怎样第一次成为任务现场](img/fig-029-initial-task-stack-frame.png)

这张图从左边的"材料"看起，别一上来就盯寄存器名：入口函数和参数先进初始栈帧，`top_of_stack` 再交给 TCB，最后回到调试器里检查入口地址、参数指针和 PSP 范围是否都落在该任务的栈内。

把这条线连上，栈就从"一段数组"变成了"可恢复现场的入口"，排查也有了具体抓手。任务如果第一次运行就 HardFault，第一反应不该只是"任务函数第一行写错了"——更稳的是回头核对初始现场：入口地址是否落在有效代码区、参数指针是否还有效、栈顶是否按端口要求对齐、PSP 是否落在任务栈范围内。这四个问题，每一个都比"任务没起来"更能在调试器里验证。

同样，当 LED 打了 `before delay` 却迟迟不见 `after delay`，也别急着下"函数没被再调用"的结论。LED 的调用链和局部现场还在它自己的栈里，Delay 只是让它离开就绪、把工作台让给别人；该追的是下面这几步到底断在哪：

| 看到的现象 | 先别急着下的结论 | 更稳的下一问 |
| --- | --- | --- |
| 有 `before delay`，没 `after delay` | LED 函数没被再次调用 | LED 是否从 delayed 回到 ready、现场是否被恢复 |
| 切换后局部变量错乱 | C 语言局部变量规则失效了 | 任务栈是否溢出、PSP 是否越界 |
| HardFault 停在恢复现场附近 | 一定是 PendSV 写错了 | 是否恢复了一份已经被破坏的栈 |

任务这一层立住了，下一节接着追一个更具体的问题：这块现场的栈顶到底被谁长期记着，任务名、优先级、在系统里的位置，又怎样围着同一个任务对象组织起来——那份"档案"就是 TCB。
