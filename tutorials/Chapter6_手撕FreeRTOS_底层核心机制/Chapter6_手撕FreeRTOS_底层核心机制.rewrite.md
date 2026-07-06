
# Chapter 6 手撕 FreeRTOS：从一个人单干，到请个工头带班

> 本文件是第六章的可读性重写稿，逐节写入、逐节验收，定稿后替换正式章节。
> 主线比喻：四个脾气迥异的"人"（LED / SENSOR / COMM / LOG）共用一张工作台（CPU）。
> 裸机是一个老师傅把活全包、排队干；FreeRTOS 就是那个自己不下场、只管调度记账的工头。

## 0 开场：四个人，一张工作台

一块很小的板子上，同时住着四个"人"，脾气各不相同。

LED 是**报平安的**：每隔 50 ms 稳稳眨一下眼，告诉外面"我还活着"。它的活最轻，却最讲一个"稳"字——晚眨一下，别人就要怀疑板子是不是死机了。SENSOR 是**掐着表干活的**：每 20 ms 采一个数据，早一点晚一点都不行，采晚了这口数据就"馊"了。COMM 是**接线员**：平时闲着，外面一来命令就得立刻回话，慢半拍对方就当你掉线了。LOG 是**记流水账的**：把运行信息从串口一个字一个字往外吐，天生就慢，也天生不着急。

四个人单看都不难，难就难在**脾气对不上**：LED 要**稳**，SENSOR 要**准**，COMM 要**快**，LOG 天生**慢**。

| 工人 | 节奏 | 它最在乎的证据 | 将来对应的 FreeRTOS 机制 |
| --- | --- | --- | --- |
| LED | 固定周期、动作轻 | 翻转时刻稳不稳 | Delay、就绪列表、调度 |
| SENSOR | 固定周期、要数据新鲜 | 采样点漂没漂 | vTaskDelayUntil、任务栈、队列 |
| COMM | 外部事件驱动、响应压力大 | 从收到事件到回话的耗时 | 优先级、队列、PendSV |
| LOG | 慢 I/O、后台处理 | 队列积压、单次输出耗时 | 队列、互斥锁、低优先级任务 |

现在问题来了：一颗单核 MCU，同一时刻只有**一双手**。这四个脾气不一的人，怎么共用这一双手？

### 0.1 裸机：一个老师傅单干，什么活都排成一队

裸机的答案很朴素——**找个老师傅，把四样活全包了**，排成一队，一件接一件地做。这就是我们再熟悉不过的那个 `while(1)`：进循环，翻一下 LED，采一次 SENSOR，问问 COMM 有没有消息，再让 LOG 打印几行，然后回到开头，周而复始。

活少的时候，这样干挺好，甚至很好懂。可只要队伍里混进一个慢手，麻烦立刻就来：**排在慢手后面的人，只能干等**——哪怕他的活又急又短。老师傅正埋头替 LOG 写那一长串流水账的时候，LED 该眨的眼、SENSOR 该采的样，都只能乖乖排在后面。一个人慢，全场都慢，这就是裸机主循环解不开的死结：**一慢俱慢**。

嘴上说没用，跑一遍看。把四个人塞进一个 `while(1)`，让 LOG 偶尔"卡壳"一下，问题立刻现形。[`v0_bare_loop`](code/v0_bare_loop/demo.c) 只留一个现象：**LOG 一忙，LED 的心跳和 SENSOR 的采样时刻，会跟着一起被推后。**

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

跑出来是这样（`log_flush` 在 `t=40` 起连卡三次，每次占住主循环 35 ms）：

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

读这份输出，别盯代码，盯时间。前 40 ms 风平浪静，LED、SENSOR 都按自己的点准时出现。可从 `t=040` 起，LOG 进入忙碌期，每卡一次，主循环的表就往后跳 35 ms——于是 LED 本该在 `t=050` 附近的那次眨眼，被硬生生拖到了 `t=075`；SENSOR 也不再是稳稳的每 20 ms 一次。

请特别留意一件事：**LED 和 SENSOR 自己一行代码都没改，也一点没变慢，纯粹是被 LOG 连累的。** 它们和 LOG 拴在同一根绳上，绳子被 LOG 一拽，所有人跟着往后挪。这根被慢手拽长的绳子，画出来就是这样：

![裸机时间线被慢日志拉长](img/fig-009.png)

先找到 LOG 那一格，再看它后面撑开的一大段空白，最后看 LED、SENSOR 原来的节拍怎样跟着整体偏移。看懂这张图，你就攥住了整章的"病根"。后面要讲的任务、Delay、队列、优先级，没有一个是凭空造出来的名词——**它们全冲着治这一个病来的：把绳子解开，别让慢手拖住所有人。**

### 0.2 RTOS：请个工头，让该等的人先歇一会儿

那 FreeRTOS 是怎么治的？先说清楚它**不是**什么：它不是变魔术，把一双手变成四双手。单核 MCU 同一时刻仍然只有一个人在台上，这点 RTOS 改不了，也没打算改。

它做的是另一件更聪明的事——**请了个工头**。这工头自己不下场干活，只管两件事：

- 谁遇上"要等"（LED 等时间、COMM 等消息、LOG 等串口把字吐完），**准他先退到一边歇着**，把工作台腾出来给别人；
- 手里拿个小本子**记账**：谁在等、等什么、谁现在有资格上台、谁到点了该被叫回来。

就这么两件事，整个局面全变了。裸机里"谁在拖谁"是一笔糊涂账，藏在时间线里看不见摸不着；有了工头这个小本子，它变成了明明白白、能查能改的东西：

> 裸机的等待，是**整条主循环一起卡死**；RTOS 的等待，是**某一个人退到一边、被登记在册**的一种状态——CPU 不必陪他空耗，工头到点了自会把他叫回来。

这本"小账"，就是这一章要一页一页翻开的东西。**`任务`，是给每个人立的规矩**——允许他中途离场、回来接着干；**`Delay`、`队列`、`优先级`，是工头记账的不同栏目**——分别记着"谁在等时间""谁在等料""谁更急着上台"。至于 `TCB`、`就绪列表`、`PendSV`、`heap_4` 这些名字，会陆续登场，但你永远可以把它们拽回四个再具体不过的问题：这个人现在**为什么没上台**？到点了**怎么还没轮到他**？台上这位**为什么被人插了队**？内存看着够，**为什么建对象却失败了**？

读法上给一句建议：第一遍顺着往下读到"全景"那一节，只抓"现象 → 机制 → demo 证据"这条主干，源码链接看到名字就行、不必逐个点开；等主干立住了，再带着具体问题回头对账 `tasks.c`、`list.c`、`queue.c`、`port.c`、`heap_4.c`。四个人会一路跟到底——机制再多，也总能落回他们中某一位的某一次卡壳。

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

这块现场，最小只要盯两个地址就够了：任务自己的**栈底**（stack base，这片现场归谁），和工头将来要恢复的**栈顶**（top of stack，从哪儿把现场取回来）。[`v1_task_stack`](code/v1_task_stack/demo.c) 把这两个地址直接打出来，还顺手把"任务入口"和"参数"预先摆进了初始现场里：

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

所以**任务栈是双重身份**：既是 C 语言调用链的空间，也是内核恢复执行流的依据。任务被切走时，关键寄存器和返回现场被压进这片栈；被恢复时，CPU 再从这里把现场取回。也正因如此，任务里一个较大的临时数组、一段深调用、一次 `printf` 格式化，都会实实在在地吃掉栈水位——它们和"现场保存"共用同一片空间。

这也解释了写任务时那条最容易搞反的直觉：任务函数**不是**会被反复从头调用的普通函数。它进循环后长期存在，Delay 或阻塞只是让出工作台，从不把函数"重启"。把这一点想通，才会理解为什么栈水位要在**高峰路径**上盯——通信高峰、日志高峰、错误处理里，往往正是调用链最深、临时缓冲最大的时候。

### 1.3 回到源码：工头怎样给新人摆好"开工现场"

demo 里"恢复 LED 栈就跑 LED"一句话带过，可真实芯片上，这一下到底**恢复了什么**？这里得先补一点 Cortex-M 的常识，不然接下来的源码会像天书——放心，只讲够用的那几句。

CPU 干活时，手边有一小把**寄存器**，你可以把它们想成 CPU 的"随身口袋"：此刻算到哪、下一条指令在哪、函数参数是什么，全临时装在这几个口袋里。所谓**保存现场**，存的就是这一把口袋；**恢复现场**，就是把某个任务当初那把口袋原样倒回 CPU。任务能从 Delay 后面接着跑，靠的正是这个。

那 LED **第一次**运行时呢？它还从没跑过，口袋里空空如也。于是**工头得在它上工前，先把一套"开工现场"替它摆进栈里**——就像新人报到那天，工位上的工具、图纸、工号都提前放好，人一坐下就能开工。这套预先摆好的口袋，就叫**初始栈帧**。

要摆哪几个口袋？对着表看，比死记名字省力得多：

| 栈里预摆的值 | 大白话它是什么 | 为什么第一次就得摆好 |
| --- | --- | --- |
| PC | 下一条要执行的指令地址 | 指向任务入口——现场一恢复，CPU 就"落"在任务第一行 |
| R0 | 传给函数的第一个参数 | 任务入口 `void task(void *arg)` 里的 `arg` 靠它送进去 |
| LR | 函数干完后的返回地址 | 任务不该 return，这里设成一个"陷阱"，真返回了立刻报错 |
| xPSR | 状态标志（进位、零标志等） | 给个合法初值，CPU 恢复后状态才不乱 |
| R4–R11 | 一组通用寄存器 | 第一次没有有效值，先占好位置，凑齐一整套现场 |

把端口层的 `pxPortInitialiseStack()`（[port.c:202](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:202)）压成伪代码，它干的就是照着上面这张表，**把口袋一个个塞进栈**：

```c
top--;  *top = xPSR;             /* 213: 状态标志，给个合法初值 */
top--;  *top = task_entry;       /* 215: PC  —— 指向任务第一条指令 */
top--;  *top = task_return_trap; /* 217: LR  —— 任务若 return 就落进陷阱 */
top -= 5;
*top = pvParameters;             /* 221: R0  —— 任务入口的第一个参数 */
top--;  *top = exc_return;
top -= 8;                        /* R4-R11 先占位 */
return top;                      /* 230: 新栈顶，交给 TCB 记住 */
```

看着一连串 `top--` 别发怵，它只是**在栈上从高地址往低地址、一格一格往下放**，把表里那几个口袋依次摆好而已。摆完，`return` 出去的那个栈顶地址，就是一张"从这里开始把现场倒回 CPU"的**书签**。demo 输出里那三个词正好对得上：`entry_slot` 是 PC，`parameter_slot` 是 R0，`top` 就是这张要交给 TCB 收好的书签。真实版本还多摆了 xPSR、LR、R4–R11，但**主线永远只有三样：入口、参数、栈顶**。

谁来招呼工头摆这一套？是创建任务时的 `prvInitialiseNewTask()`（[tasks.c:1816](reference/rtos_src/FreeRTOS-Kernel/tasks.c:1816)）：它先算好栈顶地址，再把入口、参数、栈顶交给端口层去摆初始现场。摆好的栈顶，最后落进 TCB 的第一个字段 `pxTopOfStack`（[tasks.c:377](reference/rtos_src/FreeRTOS-Kernel/tasks.c:377)）——它被特意排在结构体最前面，好让切换时那段汇编用最快的方式一把够到它。

| demo 里的动作 | FreeRTOS 源码里的证据 | 要理解的含义 |
| --- | --- | --- |
| `entry_slot = entry` | [port.c:215](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:215) 把入口放进初始 PC 位 | 第一次恢复现场，CPU 就落到任务入口 |
| `parameter_slot = parameter` | [port.c:221](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:221) 把参数放到 R0 位 | 任务入口第一次运行就能拿到 `pvParameters` |
| `return top_of_stack` | [port.c:230](reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c:230) 返回新栈顶 | 栈顶交给 `pxTopOfStack`，将来靠它恢复 |

![任务初始栈帧：函数怎样第一次成为任务现场](img/fig-029-initial-task-stack-frame.png)

这张图从左边的"材料"看起，别一上来就盯寄存器名：入口函数和参数先进初始栈帧，`top_of_stack` 再交给 TCB，最后回到调试器里检查入口地址、参数指针和 PSP 范围是否都落在该任务的栈内。

把这条线连上，**栈就从"一段数组"变成了"可恢复现场的入口"**，排查也有了具体抓手。任务如果第一次运行就 HardFault，第一反应不该只是"任务函数第一行写错了"——更稳的是回头核对初始现场：入口地址是否落在有效代码区、参数指针是否还有效、栈顶是否按端口要求对齐、PSP 是否落在任务栈范围内。这四个问题，每一个都比"任务没起来"更能在调试器里验证。

同样，当 LED 打了 `before delay` 却迟迟不见 `after delay`，也别急着下"函数没被再调用"的结论。LED 的调用链和局部现场还在它自己的栈里，Delay 只是让它离开就绪、把工作台让给别人；该追的是下面这几步到底断在哪：

| 看到的现象 | 先别急着下的结论 | 更稳的下一问 |
| --- | --- | --- |
| 有 `before delay`，没 `after delay` | LED 函数没被再次调用 | LED 是否从 delayed 回到 ready、现场是否被恢复 |
| 切换后局部变量错乱 | C 语言局部变量规则失效了 | 任务栈是否溢出、PSP 是否越界 |
| HardFault 停在恢复现场附近 | 一定是 PendSV 写错了 | 是否恢复了一份已经被破坏的栈 |

任务这一层立住了，下一节接着追一个更具体的问题：这块现场的栈顶到底被谁长期记着，任务名、优先级、在系统里的位置，又怎样围着同一个任务对象组织起来——那一页账，就是 TCB。

## 2 TCB：工头给每个人记的一页账

还记得 §0 里那个工头吗？他手里拿着个小本子记账。现在把本子翻开看看——**每个工人占一页**，页上就记着他调度时真正要用到的那几样东西：这人是谁、他的工位（现场）在哪、活有多急、此刻在哪块区待命。这一页账，就是 **TCB（Task Control Block，任务控制块）**。

别被"控制块"这三个字唬住。先不看源码，就看工头实际往每一页上记了些什么：

| 工头账本上记的 | LED | SENSOR | COMM | LOG | 记它是为了…… |
| --- | --- | --- | --- | --- | --- |
| 是谁（名字） | LED | SENSOR | COMM | LOG | 调试器、日志、Trace 里一眼认出是谁 |
| 工位在哪（栈顶） | → LED 的栈 | → SENSOR 的栈 | → COMM 的栈 | → LOG 的栈 | 轮到他时，从这儿把现场倒回 CPU |
| 活有多急（优先级） | 中 | 高 | 高 | 低 | 抢工作台时，急的先上 |
| 现在在哪（位置） | 就绪 | 等钟点 | 等料 | 就绪 | 一眼看出他此刻为什么没在台上 |

一句话：**TCB 把"一段函数代码"变成了工头眼里"这一个人"**。有了这页账，调度、切换、唤醒才谈得上；没有它，工头手上只剩一堆长得一模一样的函数地址，谁是谁都分不清。下面就把这页账拆开看：为什么非记不可，源码里又长什么样。

### 2.1 为什么不能只记住一个函数地址

上一节已经知道，任务不只是一段函数代码。麻烦在于：LED 和 LOG 完全可能用**同一个任务入口模板**，只是参数不同；SENSOR、COMM 也各带参数启动。如果内核只记得函数地址，它根本分不清"现在该恢复谁的栈""谁的优先级更高""谁正在队列里等着"。函数名还有个死穴——它看不出**当前状态**：一个入口叫 `comm_task` 写得再清楚，也说不出此刻它是在等数据、等时间还是已经 ready。

所以任务一进内核，就需要一份稳定资料把"某个入口函数"钉成"这个任务对象"。**TCB 要回答的四个问题，正好分成四组**：身份（它是谁）、现场（从哪里恢复）、调度（有多急）、位置（现在在哪个列表）。后面所有机制翻的都是这四组。

![TCB 字段按身份、现场、调度、位置分组](img/fig-005.png)

拿 LED 代进去复述一遍，图就落地了：**任务名**让调试器显示"这是 LED"，**栈顶**指向 LED 自己的现场，**优先级**决定它和 LOG、SENSOR 的竞争关系，**列表节点**说明它此刻在 ready、delayed 还是 event wait 里。能这么复述，TCB 就不再是一张字段清单，而是任务对象的证据中心。

### 2.2 四个字段先站出来

TCB 的最小形状看 [`v2_tcb`](code/v2_tcb/demo.c)，`MiniTCB` 故意只留四个字段，让身份、现场、调度、位置先各就各位：

```c
typedef struct {
    const char *name;          /* 身份：它是谁 */
    uint32_t *top_of_stack;    /* 现场：从哪里恢复 */
    unsigned priority;         /* 调度：有多急 */
    MiniListItem state_item;   /* 位置：挂在哪个列表 */
} MiniTCB;
```

```output
TCB name=LED     priority=2 top_of_stack=<addr> list_owner=LED
TCB name=LOG     priority=1 top_of_stack=<addr> list_owner=LOG
TCB is the scheduler handle: identity + stack + priority + list hook
```

四组信息压进一行：`name` 是身份，`priority` 是调度依据，`top_of_stack` 接上一节的任务栈，`list_owner` 预告这个任务将来会通过列表节点被挂进 ready、delayed 或 event wait——**上一节的栈和下一节的列表，就在 TCB 这里接上了头**。真实的 `TCB_t` 比这大得多（还要管配置项、运行统计、任务通知、mutex 优先级继承），但第一轮别贪，先确认这四类字段怎样撑起"任务是内核对象"。

同一个 LED 的 TCB，会被四种机制反复翻看：**创建**时被填好，**列表**移动时被挂到 ready/delayed，**调度**时被拿来比优先级，**PendSV** 切换时被取出栈顶。四个镜头看的是同一个对象，只是各取所需——这也解释了为什么这么多机制都要碰它。

### 2.3 回到 tasks.c：把字段接回使用它的机制

打开真实 `TCB_t`（[tasks.c:375](reference/rtos_src/FreeRTOS-Kernel/tasks.c:375)）最容易挫败——字段太多，个个都像很重要。**破解办法是不按声明顺序背，而是按"谁会读写它"分组**。和当前主线相关的就四组，其余配置字段先搁一边，等读到队列、mutex、任务通知再回补：

| TCB 字段 | 源码锚点 | 谁在用它 | 项目里能解释什么 |
| --- | --- | --- | --- |
| `pxTopOfStack` 现场 | [tasks.c:377](reference/rtos_src/FreeRTOS-Kernel/tasks.c:377) | PendSV、端口层 | 切换后从哪里恢复现场 |
| `xStateListItem` 位置 | [tasks.c:387](reference/rtos_src/FreeRTOS-Kernel/tasks.c:387) | ready/delayed/suspended 列表 | 任务此刻在哪里 |
| `xEventListItem` 位置 | [tasks.c:388](reference/rtos_src/FreeRTOS-Kernel/tasks.c:388) | queue/mutex/事件等待列表 | 任务在等哪个资源 |
| `uxPriority` 调度 | [tasks.c:389](reference/rtos_src/FreeRTOS-Kernel/tasks.c:389) | 调度器、mutex 继承 | 谁更该先拿到 CPU |

注意 `pxTopOfStack` 被特意排在结构体**第一个字段**（源码注释也强调了 THIS MUST BE THE FIRST MEMBER）——因为上下文切换的汇编要用最快的方式够到它。这不是随意的声明顺序，而是给切换路径留的近道。

![TCB 字段怎样被调度、列表、PendSV、队列使用](img/fig-030-tcb-field-usage-map.png)

读这张字段图，别从字段名往下背，**从外侧机制往回问**：PendSV 为什么要找栈顶、调度器为什么看优先级、Delay 为什么能移动任务、mutex 为什么能找到等待者。每一问都落回 TCB 的某一组字段，结构体就变成了任务对象。demo 里那个 `list_owner=LED` 也有真实出处：内核从列表里拿到的只是一个 `ListItem_t`，靠的是节点里的 **owner 指针**（创建时由 `prvInitialiseNewTask` 一并初始化）把列表项指回 TCB——这是"任务位置"回到"任务身份"的**回家路**。

有了这页账，排查就有了固定入口。而 TCB 比普通业务结构体更值得警惕的一点是：**故障点常常不是破坏点**。PendSV 恢复现场时崩了，表面在切换，真凶可能是更早某个任务数组越界、把相邻 TCB 的栈顶字段写坏了。所以排查要沿时间往回看，也要按四组证据分开查：

| TCB 证据 | 能解释的问题 | 项目里怎样观察 |
| --- | --- | --- |
| 任务名 | 调试器里认出是谁 | 任务列表、日志前缀、Trace 名称 |
| 栈顶字段 | 恢复现场是否有根 | PSP 范围、栈水位、HardFault 现场 |
| 优先级字段 | 为什么被选中或被压住 | 调度日志、ready 集合、继承前后优先级 |
| 列表节点 | 任务到底在哪里 | ready / delayed / event wait 位置 |

**TCB 坏了，调度、列表、现场恢复会被一起牵连**，所以遇到"任务像随机失踪"的现象，要把 TCB 周边内存、栈溢出、数组越界、错误指针都拉进排查范围。下一节就顺着"列表节点"往下走：任务到底挂在哪里，为什么"它在哪个列表"比"它是不是卡住了"更有用。

## 3 内核列表：任务在系统里的位置地图

§2 的账本里有一栏叫"现在在哪"。这一节就专门放大这一栏——工头到底把每个人搁在哪儿，又凭什么这么搁。

任务没输出，不代表它消失了。工头的车间里有**三块待命区**：**候场区**（ready，随时能上台）、**等钟点区**（delayed，在等一个时间点到）、**等料区**（event wait，在等队列里的数据、或某把工具的钥匙）。一个工人没在台上，先看他站在哪块区，比笼统一句"他没干活"有用得多。而这三块区，落到内核里就是三条**列表**。

### 3.1 "他在哪块区"，比"他卡住了"值钱

一个任务半天没动静，脱口而出的往往是"它卡住了"。可这话，就像工头在车间里喊一嗓子"这人怎么不见了"——喊完还是不知道上哪儿找他。**换成"他在哪块区"，下一步立刻就清楚了**：在候场区，就问调度（凭什么还没轮到他上台）；在等钟点区，就问时钟（Tick 到没到他那个点）；在等料区，就问谁负责给他送料、递钥匙。同样一句"没动静"，三块区对着三条完全不同的追查线。

这里有个容易被略过、却是整章地基的点：**工人本人，和他站在哪块区，是两码事**。§2 的账本里"现在在哪"之所以单占一栏，TCB 里之所以挂着一个"列表节点"，就是要把这两件事分开记——人是人（TCB），位置是位置（列表）。分开记，工头才能既一眼认出这是谁、又随时找得到他在哪。而所谓任务"换了个状态"，说到底就是**工头把他从一块区，挪进了另一块区**。

![工头在候场、等钟点、等料三块区之间挪人](img/fig-011.png)

看懂这张走位图，排查不动的任务就不必瞎猜：先认出是谁（找到 TCB），再看他站哪块区（在哪条列表）；区一定，下一步该问调度、问时钟还是问送料，也就跟着定了。

### 3.2 demo：看工头当面挪几次人

[`v3_kernel_list`](code/v3_kernel_list/demo.c) 就干一件事——让工头当着我们的面，把同一个工人从一块区挪到另一块区：

```c
list_insert_front(&ready, &led);
list_insert_front(&ready, &sensor);
list_insert_front(&ready, &log);

list_remove(&ready, &sensor);
list_insert_front(&delayed, &sensor);   /* SENSOR 去等时间 */

list_remove(&ready, &log);
list_insert_front(&event_wait, &log);   /* LOG 去等事件 */
```

```output
move LED    -> ready
move SENSOR -> ready
move LOG    -> ready
ready: LOG SENSOR LED
remove SENSOR <- ready
move SENSOR -> delayed
remove LOG    <- ready
move LOG    -> event_wait
ready: LED
delayed: SENSOR
event_wait: LOG
```

**别把它当链表操作读，当"走位"读**：每一行 `move`，都是工头把某个工人挪进了某块区。前三行，三个人先都进候场区；接着 SENSOR 被挪去等钟点区（它要等下一次采样时间到），LOG 被挪去等料区（它在等一个事件）。末尾那三行，就是此刻的车间一览：候场区只剩 LED，等钟点区是 SENSOR，等料区是 LOG。这张快照比一句"系统卡了"有用太多——它直接告诉你，下一步该去催谁上台、该给谁送料。

### 3.3 回到 list.c：挪人，其实就是换一块牌子

那"把工人挪块区"，落到代码里到底是什么动作？说穿了朴素得很：**每个工人身上挂着一块牌子，写着"我归哪块区"**——挪区，就是改这块牌子，再顺手给两块区的人数各自加减一下。所以读 `list.c` 不必当数据结构课，**只追三件事：谁把工人挂进一块区、谁把他摘出来、那块牌子怎样从区里指回工人本人**。

| 读什么 | 源码锚点 | 先抓住什么 |
| --- | --- | --- |
| 列表项结构 `ListItem_t` | [list.h:144](reference/rtos_src/FreeRTOS-Kernel/include/list.h:144) | `pxNext/pxPrevious/pxContainer` 说明它在哪个列表 |
| 列表结构 `List_t` | [list.h:172](reference/rtos_src/FreeRTOS-Kernel/include/list.h:172) | 一个列表怎样保存尾节点、索引和数量 |
| 有序插入 `vListInsert` | [list.c:139](reference/rtos_src/FreeRTOS-Kernel/list.c:139) | 任务或超时节点怎样进入某个位置 |
| 移除节点 `uxListRemove` | [list.c:217](reference/rtos_src/FreeRTOS-Kernel/list.c:217) | 任务怎样离开当前位置，列表数量怎样变 |

把源码压成动作骨架，`move`/`remove` 到底发生了什么就一目了然：

```c
/* insert：进某块区 */
item->pxContainer = list;        /* 挂上牌子：我归这块区 */
list->uxNumberOfItems++;         /* 这块区人数 +1 */

/* remove：离开这块区 */
list = item->pxContainer;
item->pxContainer = NULL;        /* 摘掉牌子 */
list->uxNumberOfItems--;         /* 这块区人数 -1 */
```

`move SENSOR -> delayed` 不是复制一个 SENSOR，而是**同一个任务对象的列表节点进入了 delayed list**；`remove LOG <- ready` 说明 LOG 离开 ready 候选集合，调度器不该再从 ready 里选到它。**链表函数本身很普通，它的意义全来自调用者**：Delay 调用 `vListInsert` 是为了按唤醒时间放进 delayed，队列调用它是为了把等待者挂到 event list，调度路径读 ready list 是为了找有资格的任务。

这块牌子（`pxContainer`）本身就是一条很实用的排查线：牌子写着候场区，他就该真站在候场区；被摘出来后，牌子该清空。**如果一个工人看着像同时站在两块区，或早被摘走却还被当成在册，八成是重复挪动、重复摘牌，或者内存被写坏了**——而且列表很少无缘无故坏，多半是工人对象、等待对象或内存边界先出的问题。

所以项目里说"任务 blocked"其实还不够。同样一句 blocked，COMM 可能在 `RX_QUEUE` 的接收等待列表上（外部事件没来），可能已被唤醒回 ready 却被更高优先级压住，也可能卡在 UART mutex 的等待链上（被资源 owner 挡住）。三种都让 COMM 暂时没输出，原因却南辕北辙。**把"卡住了"翻译成"在哪个列表"，就是从凭感觉调试走向按证据调试的起点**：

| 任务位置 | 看起来的现象 | 下一步证据 |
| --- | --- | --- |
| delayed list | 周期任务暂时没输出 | wake tick、当前 tick、到期是否回 ready |
| queue event list | 消费者等不到数据 | queue count、发送方是否运行、等待超时 |
| mutex event list | 高优先级任务被挡住 | owner、waiter、持锁时间、是否发生继承 |
| ready list | 有资格却没运行 | 优先级、同级轮转、当前任务、PendSV |
| 列表节点异常 | 状态混乱或崩溃 | TCB 完整性、越界写、栈水位 |

位置清楚了，源码就不再是一堆函数名，而是一条移动路线：进入列表、离开列表、回到 ready、再等调度。可再往前倒一步会发现一个更基本的前提——**任务得先"成为对象"、并被放进某个列表位置，后面的调度和唤醒才谈得上**。下一节就看这一步：任务创建，到底把函数、栈和 TCB 组装成了什么。
