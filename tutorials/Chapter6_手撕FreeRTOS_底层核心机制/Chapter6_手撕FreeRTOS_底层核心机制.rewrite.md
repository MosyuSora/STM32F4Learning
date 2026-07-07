
# Chapter 6 手撕 FreeRTOS：车间工头速成指南

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



## PART1 任务调度

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

### 1.3 回到 port.c：第一份"开工现场"是怎么摆出来的

demo 里"恢复 LED 栈，LED 就开跑"一句话带过，可真到芯片上，这一下**恢复的到底是什么**？这一节把端口层那段最唬人的初始化，一格一格拆开看——毕竟是手撕源码，得抠到字段。

#### 1.3.1 先认识"现场"：CPU 手边那把随身口袋

CPU 干活时，手边有一小把**寄存器**，把它们想成 CPU 的"随身口袋"就行：此刻算到哪（`PC`）、函数参数是什么（`R0`）、中间结果搁哪（`R1`–`R12`）、栈用到哪（`SP`）、状态标志怎样（`xPSR`）——全临时揣在这几个口袋里。

所谓**保存现场**，存的就是这一把口袋的内容；**恢复现场**，就是把某个任务当初那把口袋原样倒回 CPU。§1.2 说 LED 能从 Delay 后面接着跑，靠的正是这个：它的口袋被完整收着，轮到它时再倒回来，CPU 就"变回"了 LED。**这一节要解决的，是一个更早的问题——LED 从没跑过，它那把口袋里第一次该装什么？**

#### 1.3.2 第一次上工，口袋得由工头预先塞好

LED 第一次上台时，口袋空空如也。于是**工头得在它上工前，先替它把一套"开工现场"塞进栈里**——像新人报到，工位上的工具、图纸、工号提前摆好，人一坐下就能开干。这套预塞的口袋，就叫**初始栈帧**。塞哪几个、为什么，对着表看：

| 口袋 | 大白话 | 第一次为什么得塞它 |
| --- | --- | --- |
| `PC` | 下一条指令地址 | 塞任务入口地址——现场一恢复，CPU 就"落"在任务第一行 |
| `R0` | 第一个参数 | 任务入口 `void task(void *arg)` 的 `arg` 靠它送进去 |
| `LR` | 返回地址 | 任务不该 return，塞一个"陷阱"地址，真返回了立刻报错 |
| `xPSR` | 状态标志 | 塞个合法初值（含 Thumb 位），CPU 恢复后状态才不乱 |
| `R4`–`R11` | 通用寄存器 | 第一次没有效值，先占位，凑齐一整套现场 |

#### 1.3.3 pxPortInitialiseStack：照单把口袋一格格压进栈

真正干这活的是 `pxPortInitialiseStack()`（[port.c:202](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L202)）。压成伪代码，它就是照着上表把口袋一个个往栈里塞：

```c
top--;  *top = xPSR;             /* 213: 状态标志，含 Thumb 位 */
top--;  *top = task_entry;       /* 215: PC  —— 任务第一条指令 */
top--;  *top = task_return_trap; /* 217: LR  —— return 就落进陷阱 */
top -= 5;                        /*      跳过 R12, R3, R2, R1 */
*top = pvParameters;             /* 221: R0  —— 任务入口第一个参数 */
top--;  *top = exc_return;
top -= 8;                        /*      R11…R4 占位 */
return top;                      /* 230: 新栈顶，交给 TCB 收好 */
```

一连串 `top--` 别发怵，它只是**在栈上从高地址往低地址、一格一格往下压**。但这里藏着整段最关键、也最容易被略过的一个"为什么"——**这些口袋的摆放顺序，不是随便定的，而是照着"硬件在中断时会怎样压栈"一比一伪造的**。源码注释原话就是：

> `Simulate the stack frame as it would be created by a context switch interrupt.`（伪造一个"仿佛刚被上下文切换中断打断"时的栈帧。）

换成工头的话：**工头给新人摆的这份开工现场，是故意摆得跟"一个老员工干到一半、被叫下台时留下的半成品现场"一模一样。** 这么做的好处极大——这样一来，"第一次让新人上台"和"以后每一次换班让人接着干"，走的就是**同一套"恢复现场"的流程**（§6 会看到的那段 PendSV 汇编），内核**不用为"第一次"单开一套小灶**。`xPSR` 摆最高地址、`PC`/`LR` 紧随其后、`R0` 落在硬件出栈会读到的那一格……每一格的位置，都对齐了硬件异常返回时自动弹栈的次序。

demo 里打印的那三个词，正好对上这份栈帧的三处关键：

| demo 输出 | 落到栈帧哪一格 | 源码 |
| --- | --- | --- |
| `entry_slot` | `PC` 位（恢复后 CPU 落到任务入口） | [port.c:215](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L215) |
| `parameter_slot` | `R0` 位（入口第一次就拿到 `pvParameters`） | [port.c:221](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L221) |
| `top` | 压完后返回的新栈顶 | [port.c:230](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L230) |

#### 1.3.4 摆好的栈顶，最后交给谁收着

谁来招呼工头摆这一套？是创建任务时的 `prvInitialiseNewTask()`（[tasks.c:1816](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L1816)）：它先算好栈顶地址，再把入口、参数、栈顶递给端口层去摆初始现场。而 `pxPortInitialiseStack` 压完栈 `return` 出来的那个**新栈顶**，是这份现场的唯一"书签"——最后被收进 TCB 的第一个字段 `pxTopOfStack`（[tasks.c:377](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L377)）。§2.3 会看到，它之所以排在结构体**第一个**，正是为了让将来换班那段汇编一把就抓到它。

![任务初始栈帧：一份伪造成"被中断打断"的开工现场](img/fig-029-initial-task-stack-frame.png)

到这儿，栈就从"任务名下的一段数组"，变成了**一份可以被随时倒回 CPU 的现场**，栈顶那张书签则夹在 TCB 里等着。任务这层就立住了。可这份现场的书签既然要长期收在 TCB 里，那 TCB 到底还替每个工人记着些什么——名字、优先级、此刻在哪块区？下一节这页账，一栏栏摊开看。

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

所以任务一进内核，就得给它建一份稳定档案，把"某个入口函数"钉成"这一个人"——这份档案就是 TCB，开头那张账本记的，正是它最要紧的四栏。

![TCB 字段按身份、现场、调度、位置分组](img/fig-005.png)

这张图就是账本那四栏的可视化：记住"是谁、工位在哪、多急、在哪块区"这四组，远比记住几十个字段名有用——**下面每一栏，我们都要回到真源码里，看它为什么长这样、为什么摆这儿。**

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

### 2.3 回到 tasks.c：这页账，真身长什么样

打开真实的 `TCB_t`（[tasks.c:375](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L375)），会看到一长串字段。别被吓到——大半是 `#if config...` 裹着的**选配栏**（trace、任务通知、TLS 之类，用到某功能才占位）。把选配栏遮掉，剩下的**主干**恰好就是我们那页账的几栏：

```c
typedef struct tskTaskControlBlock {
    volatile StackType_t *pxTopOfStack;   /* 工位：现场存取入口 —— 务必排第一个 */
    ListItem_t  xStateListItem;           /* 在哪块区：候场 / 等钟点 / 挂起 */
    ListItem_t  xEventListItem;           /* 在哪块区：等料（队列 / 锁） */
    UBaseType_t uxPriority;               /* 有多急：当前优先级 */
    StackType_t *pxStack;                 /* 柜子底：栈的起始地址 */
    char        pcTaskName[ configMAX_TASK_NAME_LEN ];  /* 是谁：名字 */
    #if ( configUSE_MUTEXES == 1 )
        UBaseType_t uxBasePriority;       /* 本来多急：还锁后复原用 */
    #endif
    /* …… 一大串 #if config 选配字段 …… */
} tskTCB;
```

真正值得琢磨的，是它**为什么这么摆**——每一处安排背后都有个很实在的理由。

#### 2.3.1 `pxTopOfStack` 凭什么排第一个字段

源码注释甚至用大写吼了一句 `THIS MUST BE THE FIRST MEMBER`。因为换班那段汇编（下一次 §6 会看到）要从 TCB 里**飞快地抓栈顶**——排在第一个，偏移量就是 0，一条 `ldr r0, [r2]` 直接命中，省掉"加偏移"那一步。

这条路径有多烫手？**每一次 tick 抢占、每一次阻塞和唤醒都要走一遍，一秒钟成百上千次**——省下的那一条指令，是实打实乘以千次的。正因这块地皮金贵，规矩还不止一条：开启 MPU 时，那个 `xMPUSettings` 字段被硬性规定**只能屈居第二**（源码同样大写标了 `THIS MUST BE THE SECOND MEMBER`）。**结构体最前面这一两格，是专门给切换快路留的保留席**，谁也不许插队。

#### 2.3.2 为什么要两块牌子（`xStateListItem` + `xEventListItem`）

因为一个工人**可能同时挂在两块区**。设想 COMM 去传送带那头等一批料、但只肯等 100 ms：这一刻它**既在等料区**（挂在传送带的等待名单上）、**又在等钟点区**（挂在延时名单上，等那个超时时刻），谁先到算谁的。可 §3 讲过——**一块牌子（一个 `ListItem_t`）同一时刻只能进一条列表**。要同时挂两处，身上就得有两块牌子。源码把这俩字段并排摆着，正是为这个。

#### 2.3.3 为什么优先级要留两栏（`uxPriority` 和 `uxBasePriority`）

而且 `uxBasePriority` 还被 `#if ( configUSE_MUTEXES == 1 )` 单独裹着——**没开互斥锁，压根没有这一栏**。这栏是给下一步的"钥匙"（§9 互斥锁）准备的：占着钥匙的低优先级工人会被**临时提级**赶工、还锁后再**复原**。这就得两个格子：`uxPriority` 记"现在多急"（可能被抬高），`uxBasePriority` 记"本来多急"（照它填回）。**一个编译开关，精准地说明了"这栏只为那个机制而生"**——用不到就不占地方。

#### 2.3.4 名字为什么用定长数组、不用指针

`pcTaskName` 是 `char[configMAX_TASK_NAME_LEN]`，创建时把名字**整个抄进账页**，而不是记一个指向外部字符串的指针。这样这页账**自带身份、不依赖外头那个字符串还在不在**，调试器随时读得出"这是 LED"。

#### 2.3.5 账页和牌子，怎样互相找到对方

最后一处，补上"账本"和"三块区"之间的回路。§3 会讲到：工头把工人挂进某块区，靠的就是账页里那两块牌子（`xStateListItem`/`xEventListItem`）——**账页借牌子，把自己挂进区**。可反过来呢？工头从区里随手捡起一块牌子，怎么知道是谁的？

答案是**牌子背面写着工号**。创建任务时，`listSET_LIST_ITEM_OWNER` 把牌子的 `pvOwner` 指针指回它所在的这页 TCB。于是账页和牌子结成一条**双向链**：账页 →（牌子）→ 区，区 →（`pvOwner`）→ 账页。从任一头都能一步跳到另一头——所以调度器从候场区捞出一块牌子，翻个面就立刻知道该给谁派活，不用满仓库找人。

```mermaid
flowchart LR
    subgraph TCB["LED 的一页账 · TCB"]
      S["pxTopOfStack · 工位"]
      B1["xStateListItem · 牌子①"]
      B2["xEventListItem · 牌子②"]
    end
    S --> STK["LED 的栈（柜子·现场）"]
    B1 --> DLY["等钟点区 delayed"]
    B2 --> EVT["等料区 event"]
    EVT -. "pvOwner · 工号（区→账页）" .-> TCB
```

![TCB 字段怎样对应到工位、区、优先级、名字](img/fig-030-tcb-field-usage-map.png)

一句话收束：**`TCB_t` 绝不是字段的随机堆叠，而是把"是谁、在哪块区、多急、工位在哪"这几件事，按各个机制取用它的方式精心排布的一页账。** 读它的正确姿势，是照着"哪个机制会翻哪一栏"去认，而不是从头背到尾。下一节就顺着账里那两块牌子往下走，看任务究竟怎样在三块区之间移动。

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

### 3.3 回到 list.c：一块牌子、一块区、两个动作

"把工人挪块区"，落到代码里就是改牌子。可这套牌子和区，源码设计得极其精巧——手撕一下，你会发现好几处"为什么这么写"都藏着巧劲。

#### 3.3.1 一块牌子（ListItem_t）上刻了哪些字段

先看牌子本身，`ListItem_t`（[list.h:144](../../reference/rtos_src/FreeRTOS-Kernel/include/list.h#L144)）遮掉体检字段，只剩四样：

```c
struct xLIST_ITEM {
    TickType_t             xItemValue;   /* 牌子上的"号"：多数时候用来排序 */
    struct xLIST_ITEM     *pxNext;       /* 前一位、后一位：双向手拉手 */
    struct xLIST_ITEM     *pxPrevious;
    void                  *pvOwner;      /* 牌子背面的工号：指回 TCB */
    struct xLIST          *pxContainer;  /* 牌子正面：我此刻归哪块区 */
};
```

对着引子一栏栏认：`pxContainer` 是牌子**正面**——写着"我归候场区/等钟点区/等料区"；`pvOwner` 是牌子**背面的工号**——指回它所属的 TCB（§2.3 那句"牌子背面写工号"的真身）。这俩一正一反，**把"人"和"他的位置"双向锁死**：从区里捞到一块牌子，翻背面就知道是谁；拿到一个人，看他牌子正面就知道他在哪。

`pxNext/pxPrevious` 是**双向**的——不是单链表。为什么？因为工头经常要把一个人**从队伍正中间**直接抽走（比如 COMM 在等料区排到一半，料来了就得立刻拎出来）。双向手拉手，抽人时左右两位一牵手就接上了，**不用从头找一遍，一步到位**。

`xItemValue` 是牌子上写的一个"号"，多数时候拿来**排序**——这个号，下面马上会用到。

#### 3.3.2 一块区（List_t）怎么组织：一个永远排最后的"假人"

再看区，`List_t`（[list.h:172](../../reference/rtos_src/FreeRTOS-Kernel/include/list.h#L172)）就三样有用的：

```c
typedef struct xLIST {
    UBaseType_t   uxNumberOfItems;   /* 这块区几个人 */
    ListItem_t   *pxIndex;           /* 巡查游标：上次点到谁 */
    MiniListItem_t xListEnd;         /* 哨兵：一个号最大、永远排最后的"假人" */
} List_t;
```

`xListEnd` 是全章最值得拍案的一处设计。**每块区里都常驻一个"假人"哨兵**，它的 `xItemValue` 被设成**最大可能值**，所以永远排在队尾。图它什么？——**图省掉一切边界特判**。有了这个永不消失的假人，区就永远不是"空指针"：往里插人、从里抽人，代码都不必再为"这区是空的吗""插的是队头吗""删的是最后一个吗"写一堆 `if`。尤其有序插入时，新人一路往后比号，**比到假人这儿必然停下**（谁也大不过它），天然就是循环的终点。一个假人，把链表最烦的边界情况全抹平了。

`pxIndex` 是把**巡查游标**。轮到从候场区选人上台时，工头不是每次都从头点名，而是**从上次停的地方接着往下点**——于是同样急的几个人，就一轮一轮地轮着来。§5 说的"同优先级轮转"，真身就是这根游标在候场区里一圈圈地走。

> 🎨 **配图提示词**（AI 生成后替换为下方图片）

```text
一张干净扁平的技术示意图，浅色背景，单一蓝色强调色。主题：FreeRTOS 就绪列表
的环形双向链表。画一条水平排开的环形链：三个任务节点（标注 LED、SENSOR、LOG）
外加一个特殊的"哨兵"节点（醒目标注 xListEnd：号最大、永远排队尾）。相邻节点之间
用双向箭头相连（标注 pxNext / pxPrevious），并从哨兵绕回队头形成闭环。一个游标
箭头指向其中一个节点（标注 pxIndex：上次点到这里）。整体像"候场区排队"，节点画成
小方卡片，中文标注，无阴影无渐变，线条清爽。
```


#### 3.3.3 挪人：vListInsert 与 uxListRemove 两个动作

有了牌子和区，"挪人"就落到两个函数上。压成骨架：

```c
/* vListInsert：进某块区（list.c:139）*/
按 xItemValue 从小到大，找到该插的位置;   /* 走到哨兵必停 */
把新牌子的 pxNext/pxPrevious 接进链;
item->pxContainer = list;                  /* 挂正面：我归这块区 */
list->uxNumberOfItems++;

/* uxListRemove：离开这块区（list.c:217）*/
item->pxPrevious->pxNext = item->pxNext;   /* 左右两位牵手，一步抽离 */
item->pxNext->pxPrevious = item->pxPrevious;
item->pxContainer = NULL;                  /* 摘牌 */
list->uxNumberOfItems--;
```

`vListInsert`（[list.c:139](../../reference/rtos_src/FreeRTOS-Kernel/list.c#L139)）最关键的是**"有序"**二字——它按 `xItemValue` 升序插。等钟点区正是靠这个：每个人牌子上的号，写的就是他"该被叫醒的那个 tick"，于是**最早到点的人永远排在队头**。这样 §7 里挂钟每响一下，工头只需**瞄一眼队头**就知道有没有人到点，根本不用翻整队——一个排序，把每次 tick 的开销从"看所有人"压到了"看一个人"。

`uxListRemove`（[list.c:217](../../reference/rtos_src/FreeRTOS-Kernel/list.c#L217)）则把 3.3.1 那个双向指针用满：左右两位一牵手，任意位置的人**一步抽离**，再摘牌、人数减一。

回到 demo：`move SENSOR -> delayed` 不是复制一个 SENSOR，而是**同一个人的那块牌子，正面从"候场"改成了"等钟点"**；`remove LOG <- ready` 是 LOG 的牌子被摘下、从候场区的链里牵走。**链表函数本身普通得很，精巧全在这套牌子—哨兵—游标的组织上**：它让"一个人在哪、这块区有谁、下一个轮到谁"三件事，都变成了 O(1) 或近乎白送的操作。

牌子和区都摸透了。可我们一直默认工人"本来就在那儿"——**他当初是怎么被招进来、第一次站进候场区的**？下一节看任务创建。

## 4 任务创建：给新人办入职，不等于让他上台

前三节，我们把一个"工人"拆成了三样东西：他的**工位**（栈，§1）、他那**页账**（TCB，§2）、他站的**区**（列表，§3）。这一节要做的，是把这三样东西**装到一起**，变成工头真能调度的一个人。干这件事的，就是 `xTaskCreateStatic()`——把它想成**给新人办入职**：备齐工位、建好账页、摆好开工第一天的现场，最后把名字往花名册的"候场区"里一记。

这里有个新手几乎都会踩的误会，先用一句话钉死：

> **办完入职，不等于已经上台干活。** `xTaskCreateStatic()` 返回成功，只说明这个人齐活了、进了候场区待命；他哪一刻真站上工作台，得等工头派活（调度）、甚至等开工铃响（启动调度器）。

### 4.1 "创建成功"到底成了什么

调用 `xTaskCreateStatic()` 时，表面看只是递进去入口函数、任务名、栈、参数、优先级五样东西。可内核在背后跑的是一条**装配线**：拿栈摆好开工现场（§1 那套寄存器口袋），把名字、优先级、栈顶填进账页（§2 的 TCB），最后把这页账挂进候场区（§3 的 ready 列表）。

所以"创建成功"和"任务在跑"根本是两码事。**创建成功 = 装配完成、人在候场区**；至于他什么时候打印第一条日志，还得看三件事：调度器启没启动、优先级够不够高、他一上台会不会又立刻去等钟点或等料。把这条边界记牢，后面一大堆"任务怎么没起来"就能分出层次，而不是一股脑赖到"入口函数写错了"。

### 4.2 demo：办完入职，人却还在候场区待命

[`v4_static_task_create`](code/v4_static_task_create/demo.c) 专门把这个误会拆开。它给 LED、LOG 各办一次入职，然后你会发现——**代码从头到尾没调用过任何任务入口函数**：

```c
static MiniTCB *mini_xTaskCreateStatic(TaskEntry entry, const char *name, void *parameter,
                                       unsigned priority, uint32_t *stack_base,
                                       unsigned stack_words, MiniTCB *tcb, ReadyList *ready) {
    tcb->name = name;
    tcb->entry = entry;                          /* 账页：入口、参数、优先级 */
    tcb->parameter = parameter;
    tcb->priority = priority;
    tcb->top_of_stack = &stack_base[stack_words - 1];   /* 栈顶：开工现场从这儿恢复 */
    ready_insert(ready, tcb);                     /* 送进候场区，仅此而已 */
    return tcb;
}
```

```output
created LED    priority=2 top=<addr> -> ready list
created LOG    priority=1 top=<addr> -> ready list
ready list after creation:
  LOG is ready but not necessarily running
  LED is ready but not necessarily running
```

重点在最后两行那句 **`ready but not necessarily running`**——人办好入职、进了候场区，但一个都还没上台。这正是这个 demo 最想让你记住的：**候场区里"有资格"，和工作台上"正在干"，中间还隔着工头派活和真正切过去两步**。

放回项目里，这句话就是启动阶段的分界线：`xTaskCreateStatic()` 返回成功、任务入口却没打印第一条日志，**先别急着改入口函数**——先确认 TCB 和栈材料是不是长期有效、人是不是真进了候场区，再看调度器启没启动、是不是被更急的任务一直压着。

### 4.3 回到 tasks.c：入职三步，步步有讲究

入职落到源码，就是三个函数接力。可别以为是三句大白话——手撕进去，每一步里都塞着"为什么非这么写不可"的讲究。

#### 4.3.1 三步接力：哪一步都没让人真的上台

| 入职环节 | 源码入口 | 干的活 |
| --- | --- | --- |
| 收材料 | [`xTaskCreateStatic()` `L1332`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L1332) | 接住应用给的静态栈、TCB、入口、参数、优先级 |
| 建账页 + 摆现场 | [`prvInitialiseNewTask()` `L1816`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L1816) | 填 TCB，并调 [`pxPortInitialiseStack()` `L202`](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L202) 摆好开工现场 |
| 进候场区 | [`prvAddNewTaskToReadyList()` `L2052`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L2052) | 把新人挂进就绪表——**只给资格，不给工作台** |

压成骨架，一眼就看出**哪一步都没有真的让任务跑起来**：

```c
prvInitialiseNewTask(entry, name, priority, parameter, pxNewTCB);   /* 填账页、摆现场 */
prvAddNewTaskToReadyList(pxNewTCB);                                 /* 送进候场区 */
return (TaskHandle_t) pxNewTCB;                                     /* 交回句柄，人却还没上台 */
```

![任务创建的四步组装线](img/fig-012.png)

真正的手艺，藏在中间那步 `prvInitialiseNewTask` 里。

#### 4.3.2 prvInitialiseNewTask：那些不起眼却讲究的动作

翻开 `prvInitialiseNewTask`（[tasks.c:1816](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L1816)），会看到一串平平无奇的赋值。可每一行几乎都在解决一个具体问题：

- **先把柜子刷一层漆（栈染色）**：`memset(pxStack, 0xA5, …)`——工位还没人用，先把整个栈刷成同一种漆（`0xA5`）。图啥？将来看这层漆被磨掉了多少，就知道这人干活时最深压到过哪一格——**栈到底用了多少、会不会溢出，一眼可量**。这就是"栈高水位"能测出来的根。
- **栈顶不是随手取的，要对齐**：算栈顶时，不是简单取数组最后一格，而是往下**抹到 8 字节对齐**（Cortex-M 的硬性要求），还配了个 `configASSERT` 兜底。对不齐，将来异常压栈就会出乱子。
- **名字一个字一个字抄**（正是 §2.3.4 那条）：逐字拷进 `pcTaskName`，遇 `\0` 就停，末尾还**强制补一个 `\0`**——哪怕你名字起超长，也保证不越界、读得出。
- **优先级当场夹紧**：`configASSERT` 先挡一道，再把超界的值夹到 `configMAX_PRIORITIES - 1`。为什么这么谨慎？因为**这个优先级马上要拿去当数组下标**索引就绪表（§5 就见到）——下标一越界就是灾难。
- **给两块牌子建档**：`vListInitialiseItem` 把 `xStateListItem`、`xEventListItem` 各初始化好，再用 `listSET_LIST_ITEM_OWNER` 把牌子背面的工号指回这页 TCB——**§2.3.5 那条双向链，就是在这一行接上的**。

> 🎨 **配图提示词**（AI 生成后替换为图片）

```text
一张干净扁平的技术示意图，浅色背景，单一强调色。主题：任务栈的"染色"与高水位测量。
画一根竖直的栈（一列格子），创建时整列填满同一种底色并标注 0xA5（未使用）；运行后
下半段格子变成另一种颜色表示被写过，中间画一条水平"高水位线"，标注"任务用到过的最深处"。
右侧一句话注解：未被覆盖的 0xA5 越少，说明栈越吃紧。中文标注，无阴影无渐变。
```

#### 4.3.3 进候场区，为什么裹在临界区里

材料齐了，最后 `prvAddNewTaskToReadyList`（[tasks.c:2052](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L2052)）才把新人挂进候场区。注意它**整段裹在临界区里**（`taskENTER_CRITICAL`）——源码注释写得明白：别让中断在列表更新到一半时来碰它。

为什么这么小心？因为**就绪表是工头（调度器）和中断都会伸手去碰的公共账**：挂一个人要改好几个指针、还要给任务总数 `uxCurrentNumberOfTasks` 加一。要是挂到一半被中断打断、它又恰好来读这张表，就会读到个残缺的半成品。临界区把这几下锁成"一口气做完"。

顺带，这一步还会在两种情况下先把 `pxCurrentTCB` 指向新人：**这是头一个被创建的任务**，或者**调度器还没开工、而这人比当前记着的更急**。于是开工铃一响（§6），第一个被扶上台的，正好是最急的那位。

一句提醒收尾：**静态创建把材料的生命周期交给了你**——那个栈数组和 TCB 缓冲区必须长期活着，绝不能是某个函数里一返回就失效的局部变量，否则人还在候场区，工位和账页却已被回收。

新人办完入职、进了候场区，可工作台只有一个——**工头到底派谁上台**？下一节，调度。

## 5 调度：候场区好几个人，工头派谁上台

上一节，新人办完入职、进了候场区。可候场区常常不止站着一个人，而单核工作台一次只容得下一位。于是那个老问题终于摆到台面上：**工头到底凭什么挑谁上台？**

规则其实很朴素：**谁急，谁先上**（优先级高的先）；**一样急的，轮流来**（同级轮转）。至于还在等钟点区、等料区的人，这轮压根不参与——他们又不是不想上，是还没到能上的时候。

> **ready 是"有资格上台"，running 是"这一刻真被点名站上去了"。** 这俩之间，差的正是工头那一次点名。RTOS 里一大半"怎么没跑"的怪事，都卡在这两个词的缝里。

### 5.1 谁急谁先上——优先级不是"谁重要"

先破一个常见误解：**优先级不是"谁重要"的情绪排序，而是"谁更不能晚"的工程约束**。LOG 对定位故障当然重要，但它天生能后台慢慢消化，晚几十毫秒天塌不下来；COMM 要接外部的话，晚一下对方就当你掉线了。所以 COMM 该比 LOG 急，和"谁更有价值"没关系。设计优先级时，问的不是"这个功能重不重要"，而是"它晚 10 ms、50 ms、100 ms，各会捅出什么娄子"。

### 5.2 demo：COMM 一急，就把 LED 挤下去

[`v5_priority_scheduler`](code/v5_priority_scheduler/demo.c) 里的"工头"就一个动作——扫一遍候场区，挑出最急的那个。注意它挑人的两条规矩，顺序很关键：

```c
if (task->state != TASK_READY) continue;        /* ① 不在候场区的，直接跳过 */
if (!best || task->priority > best_priority) {  /* ② 剩下的里挑最急的 */
    best = task;
    best_priority = task->priority;
}
```

```output
tick=0 switch_to=LED priority=2
tick=1 switch_to=SENSOR priority=2
tick=2 switch_to=LED priority=2
event: COMM becomes ready
tick=3 switch_to=COMM priority=3
tick=4 switch_to=COMM priority=3
```

一个 tick 一个 tick 地读：开头 LED 和 SENSOR 一样急（都是 2），工头就让他俩**轮流上**（tick 0/1/2）；到 tick 3，COMM 来了急活、变进候场区，优先级 3 比谁都高，工头立刻改派 COMM。而 LOG 呢？它一直在候场区待着，可它太不急（优先级 1），**从头到尾没被点过一次名**——这不是 bug，是它本该让路。

这就直接给了两条工程判断：**低优先级任务长期不上台，不一定是坏了**，可能只是更急的人一直占着台；反过来，**COMM 已经很急、却还是响应慢**，那就不能怪调度了，得往后看——是没被点名，还是点了名却没真正切过去。

### 5.3 回到 tasks.c：点名，其实只改一个指针

点名落到代码，最终就是把 `pxCurrentTCB` 指向被挑中那个人的账页。可"从一堆人里挑出最急的那个"这一下，源码用了一套非常漂亮的结构，快到**一条指令**就出结果。手撕开看。

#### 5.3.1 就绪表不是一条队，是一排"桶"

先纠正一个直觉：候场区**不是一条大队**，而是**一排桶**——`pxReadyTasksLists[ configMAX_PRIORITIES ]`，一个优先级一只桶，每只桶就是 §3 那种带哨兵的链表。优先级 5 的人进 5 号桶，优先级 2 的进 2 号桶。

这下 §4.3.2 那个悬念有了着落：**创建时为什么非把优先级夹进合法范围？** 因为**这个优先级就是桶的数组下标**——夹不住，就索引到桶数组外面去了，那是要出人命的越界。

#### 5.3.2 一步找到"最高的那只非空桶"（位图 + 一条 CLZ）

桶可能有几十只，怎么飞快找到"有人、且优先级最高"的那只？一只一只翻太慢。FreeRTOS 的招法极妙：**另存一张位图** `uxTopReadyPriority`——一个 32 位的字，第 N 位是 1，就表示"N 号桶里有人"。于是"找最高非空桶"就化简成了"找这个字里**最高的那个 1**"。

而"找最高位的 1"，Cortex-M 有一条**硬件指令 CLZ（数前导零）**专干这个。优化版直接一步算出来（[portmacro.h:171](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/portmacro.h#L171)）：

```c
/* 数一下位图前面有几个 0，就知道最高的 1 在第几位 */
uxTopPriority = 31 - CLZ(uxTopReadyPriority);
```

**不管你有 4 个还是 32 个优先级，永远一条指令出结果，O(1)。** 这一处设计还顺带解释了两件事：为什么优先级总数被限制在 **≤ 32**（一个 32 位字刚好装得下这张位图），以及源码里那句"极少有系统需要超过 10~15 个优先级"的底气从哪来。（没有 CLZ 的平台，退回通用版：从最高位那只桶往下、一只一只试空，见 [tasks.c:195](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L195)。）

> 🎨 **配图提示词**（AI 生成后替换为图片）

```text
一张干净扁平的技术示意图，浅色背景，单一强调色。主题：FreeRTOS 就绪表的分桶 + 位图选择。
左边画一排竖直的"桶"（标注优先级 0..N），有的桶里有任务卡片、有的空着。右边画一个 32 格的
位图（uxTopReadyPriority），每一位对应一只桶：桶里有人则该位为 1、空则为 0。用一个箭头标出
"最高的那个 1"，旁注 "CLZ 一条指令直接定位 → 最高优先级桶"。中文标注，无阴影无渐变。
```

#### 5.3.3 同一只桶里，轮流上台

选中的是一只**桶**，桶里可能还站着好几个同样急的人（比如 LED 和 SENSOR 都在 2 号桶）。派谁？——`listGET_OWNER_OF_NEXT_ENTRY` 在这只桶里把 §3.3.2 那根游标 `pxIndex` **往前推一格**，这次点到谁就是谁。下次再选到这只桶，游标又往前一格。于是同桶的人**雨露均沾、轮流上台**（源码注释原话：让同优先级任务"get an equal share of the processor time"）。§5 开头说的"同优先级轮转"，落到源码就是这根游标在桶里转圈。

![优先级挑人与同级轮转](img/fig-003.png)

#### 5.3.4 点名，落地就是改一个指针

选出的这个人，写进 `pxCurrentTCB`——`vTaskSwitchContext()`（[tasks.c:5120](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L5120)）忙活半天，产出就这一个指针的更新。于是全章最要命的那条分界，也就水落石出：

> **点名（选中），不等于切过去（真站上台）。** `vTaskSwitchContext()` 只把 `pxCurrentTCB` 改成新人；真正保存旧人现场、把新人现场倒回 CPU，是下一节 PendSV 的活。

工头点完名，可被点到的人还稳稳待在候场区没动——**从"点名"到"真站上台"，那惊险的一跳是怎么完成的**？这就是下一节 PendSV 的主场。

## 6 启动与 PendSV：从"点名"到"真站上台"

上一节结尾卡在一个悬念上：工头点了名（`pxCurrentTCB` 改指向 LOG 了），可 CPU 此刻还在 LED 的现场里跑——寄存器、栈指针、执行位置，全是 LED 的。**点名只是定了"下一个该谁"，真把人换上台，是另一码事。**

换班其实分两种情况，难度差得远：

- **开工第一铃**：机器是空的，刚刚打开，工头第一次派人上台。这时候台上**还没有旧人要下台**，只需把准备好的第一个人扶上去，最简单。
- **中途换班**：机器已经跑起来，台上这位正在车床上车一批零件，才车到一半就被换下来。要让他将来还能接着车，就得先把**他这半成品、连同"车到第几刀、下一刀怎么走"的进度一起收好**（存旧人的现场），再把**新上台那位上回同样没做完的活摆回台面**（恢复新人的现场），让他接着干自己那批。这一存一取，才是上下文切换的硬核，也是这一节的重头。

先讲简单的开工铃，再啃换班。

### 6.1 开工第一铃：main 把工作台交出去

裸机里，`main()` 像舞台正中那个人，从头到尾攥着执行节奏：初始化完就进 `while(1)`，所有活都在这条线上排队。RTOS 里，`main()` 的角色变了——它只负责**把台子搭好**（建任务、建队列、备资源），然后按下开工铃 `vTaskStartScheduler()`，**把工作台交给工头，自己退到幕后**。这一交，就再也不回到那个裸机主循环了。

第一次上台之所以简单，是因为**没有旧人要下台**，不必保存谁的现场，只要把创建时给第一个人摆好的那套"开工现场"（§1 里 `pxPortInitialiseStack` 预摆的初始栈帧）倒回 CPU 就行。这活由一条叫 SVC 的异常来干。[`v6_start_first_task`](code/v6_start_first_task/demo.c) 把这一交权拍成了四行：

```output
main: create tasks and start scheduler
SVC model: restore the prepared first task context
first task=COMM priority=3
main stops owning CPU; task context owns execution
```

最要紧的是最后一句 **`main stops owning CPU`**——不是说 `main()` 消失了，而是**系统的执行权已经从 main 主循环，转交给了任务世界**。此后再靠任务的等待、唤醒、调度往前推，而不是靠 main 那根 `while(1)`。真实内核里这一段是这么接力的：

| 环节 | 源码入口 | 干的活 |
| --- | --- | --- |
| 按开工铃 | [`vTaskStartScheduler()` `L3700`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L3700) | 建好 idle 任务、启动内核，转入端口层 |
| 配硬件 | [`xPortStartScheduler()` `L305`](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L305) | 配好 SysTick、PendSV 等异常优先级 |
| 扶第一个人上台 | [`prvPortStartFirstTask()` `L278`](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L278) → [`vPortSVCHandler()` `L260`](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L260) | 从 `pxCurrentTCB` 取第一个任务的栈顶，把现场倒回 CPU |

![main 把控制权交给第一个任务](img/fig-006.png)

### 6.2 中途换班：上下文切换到底在"切"什么

开工之后，真正天天发生、也最容易讲晕的，是**换班**。先说清楚换的是什么。

台上那位车零件时，"车到第几刀、进给多少、下一步怎么走"这些临时数据，全攥在 CPU 手边那一小把**寄存器**里（§1 说的"随身口袋"）。这批半成品、加上这把记着加工进度的寄存器，合起来就是他的**现场**。要把他换下去、又保证他将来能接着车，就必须**把这套现场原样收进他自己的柜子**；等他下次上台，再从柜子里把它倒回来，接着上一刀往下车。

这个"柜子"，就是每个任务自己的**栈**；而 **PSP（Process Stack Pointer）就是这个人自己柜子的钥匙**——一个专属的栈指针，指向他现场存放的位置。§2 讲过，TCB 的第一个字段 `pxTopOfStack` 存的正是这把钥匙。于是换班的骨架就三步，[`v7_pendsv_switch`](code/v7_pendsv_switch/demo.c) 把它拍得干干净净：

```output
model: scheduler already selected LOG
PendSV: save PSP=0x20001000 into LED TCB
PendSV: pxCurrentTCB LED -> LOG
PendSV: restore PSP=0x20002000 from LOG TCB
```

> 换班三步：**① 把台上 LED 的现场（PSP）锁进 LED 的柜子**（存旧人）→ **② 当前任务牌翻到 LOG**（`pxCurrentTCB` 改指向）→ **③ 从 LOG 的柜子取出他上次的现场，倒回 CPU**（恢复新人）。干这套交接的异常，就叫 **PendSV**。

为什么专门派 PendSV 这么个"交接员"，而不在时钟中断里顺手就切了？因为想换班的场合很多（时钟到点、高优先级被唤醒、当前任务主动阻塞），要是各处随手切，时机乱、还容易在中断里踩坑。把真正的切换**统一收口到 PendSV**，并让它在最低异常优先级上排队执行，时机就集中、可控了。于是分工清清楚楚：

![SysTick、调度器、PendSV 各干一段](img/fig-013.png)

**SysTick（或其他事件）负责"发现该换人了"，调度器负责"点名选谁"，PendSV 才负责"真把班换了"。** 三段分开，后面排查 HardFault 时才不会一股脑赖到 PendSV 头上。

### 6.3 为什么 PendSV 只手动存一半寄存器

这是 PendSV 汇编最唬人、也最关键的一个点，值得慢慢来。很多人第一次读 `xPortPendSVHandler` 都会卡在同一个问题上：**代码里只看见手动保存 `r4-r11`，那 `r0-r3`、`pc`、`xPSR` 这些去哪了？**

答案是：**换班的交接单，一半是"系统自动打印"的，另一半才要交接员手写补上。**

Cortex-M 有个规矩——**只要进异常，硬件会自动把一批寄存器压进当前的栈**：`r0-r3`、`r12`、`lr`、`pc`、`xPSR`。这批正是异常返回时硬件要自己用来"复位现场"的，所以它包办了。PendSV 一进来，这半张交接单已经自动打印好了，不用管。

可硬件只管这半张。另一半 `r4-r11`，按调用约定属于"谁用谁负责保住"的寄存器，硬件进异常时**不会**帮你存。偏偏任务回来后还指着它们里头的局部计算——**这半张，就得 PendSV 亲手补写**，压进这个人自己的柜子（PSP 指向的栈）。一张表看清这条分界：

| 现场 | 谁来存 | 说明 |
| --- | --- | --- |
| `r0-r3`、`r12`、`lr`、`pc`、`xPSR` | **硬件自动**（进异常时压栈） | 异常返回时硬件自己要用，不劳 PendSV 操心 |
| `r4-r11` | **PendSV 手动**（`stmdb`/`ldmia`） | 硬件不管，但任务回来要用，必须亲手补存补取 |
| `PSP` | PendSV 读出/写回 | 每个人柜子的钥匙；存进/取自 TCB 的 `pxTopOfStack` |
| `EXC_RETURN`（在 `r14`） | PendSV 连同 `r4-r11` 一起存 | 它不是普通返回地址，而是告诉 CPU"异常返回后用 PSP、回线程模式、带不带 FPU 现场" |
| `pxCurrentTCB` | 中间由 `vTaskSwitchContext` 更新 | 调度点名的结果，PendSV 照它决定"取哪个柜子" |
| `BASEPRI` | 更新 TCB 前后临时抬高/清零 | 改当前任务、就绪表这些共享账目时，别被内核级中断插队 |

![main、SVC、PSP、PendSV 关系总览](img/fig-020-svc-pendsv-psp.png)

一句话收束：**FreeRTOS 的 PendSV 不是"保存全部寄存器"，而是补齐硬件没自动存的那另一半现场。** 想通这条，再看那几行 `stmdb`/`ldmia` 就不神秘了。

### 6.4 回到 port.c：一条搬运线，两次方向反转

把 [`xPortPendSVHandler()` `L504`](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L504) 那段汇编压成 C 骨架，它就是一条规整的搬运线：

```c
old_psp = read_psp();                              /* 拿到旧人柜子的钥匙 */
save_r4_to_r11_and_exc_return(old_psp);            /* 手动补存另一半现场 */
pxCurrentTCB->pxTopOfStack = old_psp;              /* 钥匙锁进旧人的柜子(TCB) */

raise_basepri();  vTaskSwitchContext();  clear_basepri();  /* 点名：只改牌，不搬现场 */

new_psp = pxCurrentTCB->pxTopOfStack;              /* 取新人柜子的钥匙 */
restore_r4_to_r11_and_exc_return(new_psp);         /* 手动取回另一半现场 */
write_psp(new_psp);
return_via_exc_return();   /* bx r14：异常返回，硬件自动弹回那半张交接单 */
```

#### 6.4.1 前半段收旧人，后半段发新人

盯住**两次方向反转**，整段就拿下了：前半段方向是 `CPU → PSP → 旧 TCB`（把旧人现场收进他柜子），后半段方向是 `新 TCB → PSP → CPU`（把新人现场倒回台上）。中间那一下 `vTaskSwitchContext` 是分水岭。

```mermaid
flowchart LR
    subgraph SAVE["① 收旧人：CPU → 柜子"]
      A1["读 PSP<br/>（旧人钥匙）"] --> A2["手动压 r4-r11"] --> A3["新栈顶<br/>写进旧 TCB"]
    end
    A3 --> SW["② vTaskSwitchContext<br/>只翻牌·改 pxCurrentTCB"]
    SW --> B1
    subgraph REST["③ 发新人：柜子 → CPU"]
      B1["从新 TCB<br/>取 PSP"] --> B2["手动弹 r4-r11"] --> B3["写回 PSP<br/>→ bx r14"]
    end
```

#### 6.4.2 中间那两句 BASEPRI，是给调度器上的一把锁

`vTaskSwitchContext` 被 `raise_basepri()` / `clear_basepri()` 夹在中间，不是摆设。§5.3 讲过，它要翻位图、点最高桶、推游标、改 `pxCurrentTCB`——碰的全是**调度器和中断共用的那本公共账**（跟 §4.3.3 挂人进就绪表是同一类隐患）。挑人挑到一半，若被一个会调用内核 API 的中断插进来，账就乱了。

`BASEPRI` 就是那把临时锁：它把**优先级不高于 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 的中断**先挡在门外，等点完名再放开。**注意它不是关掉所有中断**——比它更紧急的中断照样能抢进来，只是那些中断被约定"不许碰内核 API"，所以碰不到这本账。这一手，既保住了挑人的原子性，又没为了这点事把最急的中断也一起憋死。

#### 6.4.3 收尾的 bx r14：不是返回，是"异常返回"

最后那句 `bx r14` 别当普通函数返回读。此刻 `r14` 里装的是 `EXC_RETURN`——一个特殊值，触发的是**异常返回**：CPU 按它回到线程模式、改用新人的 PSP、（若这人用了 FPU）连浮点现场一起恢复，然后**硬件自动**把那半张自动交接单（`r0-r3`/`r12`/`lr`/`pc`/`xPSR`）弹回寄存器。至此，新人才真正站上了台。软件补的那一半、硬件弹的那一半，在这一跳严丝合缝地拼成一整套现场——这正是 §1.3.3 说的"初始栈帧要伪造成被中断打断的样子"最终兑现的地方。

到这儿，一个任务从"是谁"到"在哪块区"、被"点名"、再"真站上台"的整条链就通了。可我们一直默认一件事没深究：**任务凭什么会主动让出工作台去"等"？** LED 说"我要等 50 ms"，这 50 ms 里 CPU 去干嘛了、时间又是谁在数？下一节就看 Delay 与 Tick。

## 7 Delay 与 Tick：工头怎么数钟点、到点叫人

LED 眨完一次眼，接下来 50 ms 它没事干，就想歇着、到点再回来。问题是：这 50 ms 里，**CPU 该干嘛？时间又是谁在数？** 这一节就答这两个问题。

### 7.1 LED 想等 50 ms，可不能占着工作台干等

最笨的办法，是让 LED 站在工作台上一直数数：`while (没到 50ms);`。可这么一来，它就成了 §0 里那个慢手——**占着工作台却不干正事，SENSOR、COMM、LOG 全被它堵在后面**，一慢俱慢的老毛病立刻重演。

聪明的办法，是让 LED 跟工头打个招呼："我这活得歇 50 ms，这段别派我。" 说完它就**主动从候场区退出来，到"等钟点区"（§3 那条 delayed 列表）挂个号**，号上写一句"到第 50 ms 叫我"。工作台立马腾出来给别人。这，就是 `vTaskDelay()` 干的事。

> **Delay 不是"睡一觉"，而是把"等时间"从"占着工作台干等"，改成"退到等钟点区、挂个号登记在册"。** 挂号的这段时间，CPU 一点不闲着，照样轮给别的能干活的人。

### 7.2 挂钟每响一下，工头就查一次谁到点了

可挂了号，谁来叫醒？——车间墙上有口**挂钟**。这口钟就是 **SysTick**：每隔固定一小格时间（比如 1 ms）"当"地响一声，这一声叫一个 **tick**。钟每响一下，工头就抬头扫一眼等钟点区：**有谁的号到点了？到点的，就把他从等钟点区叫回候场区。**

[`v8_delay_blocked_list`](code/v8_delay_blocked_list/demo.c) 把这套"挂号—挂钟—叫人"拍了下来：

```output
tick=0 LED: ready -> delayed until tick 3
tick=1 sys tick
tick=2 sys tick
tick=3 sys tick
tick=3 LED: delayed -> ready, not necessarily running yet
tick=4 sys tick
```

一格一格读：`tick=0`，LED 挂号去等钟点区，号上写"到 tick 3 叫我"；中间 `tick=1/2/3` 挂钟一声声响；到 `tick=3`，工头一看 LED 到点了，把他叫回候场区。

![Delay 进等钟点区、Tick 到点回候场区](img/fig-014.png)

但这一行的结尾藏着最要命的四个字——**`not necessarily running yet`（未必立刻就跑）**。LED 到点了，回的是**候场区**，不是工作台！他只是重新有了"上台资格"，真要上台，还得等工头派活（§5 调度）、还得完成那惊险的换班（§6 PendSV）。这正是前两节那条线在时间维度上的复现：

> **到点回候场，不等于立刻上台。** 时间到期只解决"重新有资格"，"真站上台"是调度和切换的事——中间还隔着两步。

### 7.3 回到 tasks.c：一个负责挂号，一个负责查号

真实内核里，这套动作就落在两个函数一前一后的配合上：

| 环节 | 源码入口 | 干的活 |
| --- | --- | --- |
| 挂号 | [`vTaskDelay()` `L2469`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L2469) | 把当前任务移出候场区，算好唤醒 tick，挂进等钟点区 |
| 挂钟响 | [`xPortSysTickHandler()` `L560`](../../reference/rtos_src/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c#L560) | SysTick 中断，每格时间"当"一声 |
| 查号 | [`xTaskIncrementTick()` `L4736`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L4736) | 时钟 +1，扫等钟点区，把到点的送回候场区 |
| 回候场后还得派活 | [`vTaskSwitchContext()` `L5120`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L5120) | 到点 ≠ 运行，仍要经调度选中 |

#### 7.3.1 挂号：把唤醒 tick 写进牌子，插进一支有序队

`vTaskDelay` 的核心动作，是算出**唤醒 tick = 当前 `xTickCount` + 要等的格数**，把它写进这人 `xStateListItem` 牌子上的那个"号"（`xItemValue`），再插进等钟点区。而这一插，用的正是 §3.3.3 的**有序插入**——于是等钟点区里，**最早该醒的人永远排在队头**。挂完号，`vTaskDelay` 立刻触发一次换班，把工作台让出去。

#### 7.3.2 查号的精明：一只"下次闹钟"，让绝大多数 tick 一比就过

最笨的查号法，是挂钟每响一下就把等钟点区从头翻一遍。可 SysTick 一秒响成百上千次，每次全翻，开销受不了。FreeRTOS 的精明就在这儿：它单独存一个 `xNextTaskUnblockTime`——**全场下一个该醒的时刻**，其实就是队头那人的唤醒 tick。

于是 `xTaskIncrementTick` 里每个 tick 只做一次比较（[tasks.c:4776](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L4776)）：

```c
xTickCount++;
if ( xTickCount >= xNextTaskUnblockTime ) {   /* 绝大多数 tick 这里就 false，直接过 */
    /* 只有真到点了，才去看队头，收人，再把新队头的时刻记成下次闹钟 */
}
```

绝大多数 tick，没人到点，这一句 `false` 就走完了，**链表碰都不碰**。只有真到某人的点，才去队头收人；而且因为队是有序的，**永远只看队头**——收完一个，就把新队头的唤醒 tick 记成新的 `xNextTaskUnblockTime`，不再往下翻。等钟点区空了，就把闹钟设成 `portMAX_DELAY`（几乎永不触发）。这跟 §3.3.3 "挂钟只瞄队头"是同一手：**把每个 tick 的开销压到近乎一次比较**。

顺带记住那行"请求换班"——被叫回候场的人若比台上这位更急，`xTaskIncrementTick` 只是**返回一个"需要换班"的申请**，真正切过去仍是 §6 PendSV 的活。

#### 7.3.3 到点的人，可能一次从两个名单里摘走

还记得 §2.3.2 那个"两块牌子"吗？COMM 去队列等料、但只肯等 100 ms——这一刻它**同时挂在等料区和等钟点区**。到点收人时，`xTaskIncrementTick` 把他从等钟点区摘掉后，会**顺手检查他那块 `xEventListItem` 还在不在某条等待链上，在的话一并摘掉**（[tasks.c:4822](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L4822)）。这正是"谁先到算谁的"落到代码：超时先到，就在这里把两块牌子一起清——省得料随后才来时，还去唤醒一个早已超时走人的家伙。

#### 7.3.4 为什么要两支延时链表（挂钟会绕圈）

最后一个"为什么这么写"最烧脑，却很实在：`xTickCount` 是个有限计数器，**会绕圈**（溢出回 0）。想想一个"绕圈之后才醒"的人——他的唤醒 tick 数值反而比现在还小，塞进同一支有序队里就彻底乱套了。

FreeRTOS 的解法是**备两支队**：`pxDelayedTaskList` 和 `pxOverflowDelayedTaskList`。唤醒 tick 会溢出的，先挂到 overflow 那支候着；挂钟一绕回 0，`taskSWITCH_DELAYED_LISTS()` 把两支队一交换——原来的 overflow 队转正，绕圈问题就这么被抹平了。

一句设计上的延伸：`vTaskDelay` 是**相对**延时（"这轮干完再等 50 ms"），LED 心跳用它没问题；可 SENSOR 要**每 20 ms 一个固定采样点**，相对延时会让每轮处理耗时**一点点累加、把采样点越推越偏**（§0 说的采样点漂移）。它该用**绝对基准**的 `vTaskDelayUntil`——盯"下一格在哪儿"，而不是"从现在起再等多久"。

时间这条线通了，一个任务的独角戏就演全了。可四个人凑一个班组，光会各自等时间还不够——**SENSOR 采到的数据，怎么交到 COMM 手里？** 一个人生产、另一个人消费，中间那段"传送带"就是下一节的队列。



## PART2 任务协作

## 8 队列：传送带

SENSOR 掐着表采数据，COMM 拿去往外发。一个在这头生产，一个在那头消费——**中间这批数据，怎么从 SENSOR 手上稳稳交到 COMM 手上？**

### 8.1 别在两人中间摆一块"公共黑板"

最省事的想法，是在两人中间挂一块**公共黑板**：SENSOR 采完就写上去，COMM 要发就来读。全局变量传数据，就是这么干的。可只要节奏一错开，黑板立刻乱套：COMM 还没来读，SENSOR 新的一笔就把旧的**覆盖**没了；或者 COMM 手快，读到的是一笔**上轮的旧值**；再或者两人同时动手，读到一半的**残数**。想让它不出错，就得靠一堆"你先我后"的口头约定——**又脆又累，正是 §0 那种靠自觉硬凑的老路**。

换个办法：在两个工位之间架一条**传送带**，带上留几个**格子**（这就是队列的容量）。SENSOR 把一笔数据当零件放上带子这头，COMM 从那头一件件取走。有方向、有容量、有先来后到，谁也不覆盖谁——这，就是**队列**。

### 8.2 这条传送带还会"叫人"——它可不只是个数组

到这儿，传送带听着还只是个"排好队的数组"。但队列真正的本事在下一层：**它还管人。**

带上的格子**满了**，SENSOR 手里的零件没处放，怎么办？它不会站那儿干等（又变回占着工作台的慢手），而是**退到等料区（§3 那条 event wait 列表）挂个号**，留话"有空位了叫我"。反过来，带子**空了**，COMM 没得取，同样退到等料区，留话"来料了叫我"。

更妙的是**成功放/取时那一下顺手**：

> **SENSOR 往传送带上放一个零件，不只是把 count 加一——它会顺手看一眼：那头有没有 COMM 正等着料？有的话，一并把 COMM 从等料区叫回候场区。** 反过来 COMM 取走一件、腾出个空格，也会顺手叫醒那个正等着空位的 SENSOR。

这就是队列和普通数组的分水岭：**数组只管"数据放哪儿"，队列还管"没位置时谁停下、来数据时叫醒谁"——它把数据交接和任务唤醒，绑成了同一个动作。**

### 8.3 demo：一条带子，两条线一起读

[`v9_queue`](code/v9_queue/demo.c) 把这条带子拍下来。读它的诀窍是**同时盯两条线**：`count` 是**数据线**，`waits`/`wake` 是**任务线**——只看数字，就漏掉了队列作为"叫人"对象的那一半。

```output
LOG receive -> queue empty, receiver waits
COMM send 10 -> count=1
wake receiver LOG
COMM send 11 -> count=2
COMM send 12 -> queue full, sender waits
LOG receive 10 -> count=1
wake sender COMM
COMM send 12 -> count=2
```

顺着走一遍（这里换 COMM 生产、LOG 消费）：LOG 先来取，带子是空的，它退到等料区等着；COMM 放上 10，`count=1`，**顺手把等料的 LOG 叫回候场**；再放 11，满了（容量 2）；再放 12 没处放，COMM 退到等料区；LOG 取走 10、腾出一格，**顺手叫醒等空位的 COMM**；COMM 补上 12。**每一次 send/receive，数据线和任务线都在联动**——这就是队列的全部戏眼。

![队列的数据区与两侧等待者](img/fig-004.png)

### 8.4 回到 queue.c：一条传送带的真身

`v9` 那条带子，落到源码就是 `Queue_t`。手撕开，它凭什么"既缓存数据、又管人"，就全明白了。

#### 8.4.1 Queue_t 里装了什么

遮掉选配字段，`Queue_t`（[queue.c:103](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L103)）的主干正好是"一条会叫人的带子"：

```c
typedef struct QueueDefinition {
    int8_t     *pcHead;                  /* 带子仓库的起点 */
    int8_t     *pcWriteTo;               /* 下一件放哪儿（写指针） */
    union { QueuePointers_t xQueue;      /* 当队列用时的数据 */
            SemaphoreData_t xSemaphore;  /* 当信号量/锁用时的数据 */ } u;
    List_t      xTasksWaitingToSend;     /* 等空位的发送者（按优先级排） */
    List_t      xTasksWaitingToReceive;  /* 等数据的接收者（按优先级排） */
    UBaseType_t uxMessagesWaiting;       /* 现在带上有几件 */
    UBaseType_t uxLength;                 /* 一共几格（容量，按"件"算不是字节） */
    UBaseType_t uxItemSize;               /* 每件多大（字节） */
    /* …… cRxLock/cTxLock 等选配字段 …… */
} Queue_t;
```

一眼就看到 §8.2 那句"它还管人"的真身——**两条独立的等待名单**：`xTasksWaitingToSend` 挂"等空位的发送者"，`xTasksWaitingToReceive` 挂"等数据的接收者"，而且都**按优先级排**（所以唤醒时叫醒的是最急的那个）。至于那个 `union{xQueue, xSemaphore}`——**同一副骨架，既能当队列、又能当信号量/锁**。这就是下一节"锁为什么住在 `queue.c`"的伏笔。

#### 8.4.2 环形缓冲：写指针走到头，就绕回起点

带子的格子是**一圈**接起来的。每 `send` 一件，`prvCopyDataToQueue`（[queue.c:2393](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L2393)）把数据拷到 `pcWriteTo`，写指针往前挪 `uxItemSize` 个字节；一旦挪到仓库末尾（`pcTail`），就**绕回起点 `pcHead`**。取件那头同理。为什么绕成一圈？——这样**放/取都不用挪动已有元素、也不用扩容**，永远是 O(1)：来一件写一格、走一件读一格，指针转圈跑就是了。

#### 8.4.3 为什么"按值拷贝"，不传指针

结构体注释里专门写了一句设计原则：**"Items are queued by copy, not reference."**（按值拷贝，不按引用。）`prvCopyDataToQueue` 是实打实把数据 `memcpy` **进队列自己的仓库**，而不是记一个指向发送方变量的指针。

图什么？——这样 COMM `send` 完，它那个局部变量**立刻能销毁、能复用**，队列早把内容抄了一份进自己家。要是传指针，COMM 就得保证那块数据一直活着、还得防两头同时改——又绕回 §8.1 那块"公共黑板"的老毛病了。**按值拷贝，一手交钱一手交货，两头彻底解耦。**（代价是大对象拷贝有成本，所以传大数据时人们才改传指针——但那时，指针指向的生命周期就得自己扛了。）

#### 8.4.4 数据动作和调度动作，在这里接上头

现在两条线合起来看就顺了。`xQueueGenericSend`（[queue.c:949](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L949)）拷完数据，会扭头看一眼 `xTasksWaitingToReceive`：**非空，就摘下队头那个最急的接收者、送回候场区，并请求一次换班。** 反过来，`xQueueReceive`（[queue.c:1509](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L1509)）取走一件、腾出空位后，也会去 `xTasksWaitingToSend` 叫醒最急的发送者。

![队列的数据线与任务线](img/fig-035-queue-data-task-lines.png)

**所以 `send` 从来不只是"拷字节"，它同时可能把一个任务从等料区拎回候场；`receive` 也一样。** 数据动作和调度动作在这里咬合——这正是队列配叫"任务协作对象"、而非"线程安全数组"的原因。（也顺带记住两条边界：容量只吸波峰，长期生产快于消费仍会堵；被叫回候场也只是"有资格"，真上台还得经 §5 派活、§6 换班。）

数据这条线走通了，可四个人还共用着**同一支笔**——那唯一一路 UART。LOG 正低头写着长长的流水账，COMM 突然有急事也要用它，**这支笔到底归谁、会不会俩人抢着打架？** 下一节的互斥锁，管的就是这个。

## 9 互斥锁：一支笔，一把钥匙

传送带解决了"数据怎么交"，可有些东西天生**只能一个人用**：那一支 UART 笔、那条 SPI 总线、那块正在擦写的 Flash。LOG 写日志要用笔，COMM 发响应也要用笔——**两人同时下笔，写出来的字就叠成一团谁也认不得**。这类"同一时刻只容一个人"的资源，得有个规矩管着。

### 9.1 给笔配一把唯一的钥匙

规矩很朴素：**给这支笔配一把、且只配一把钥匙**。想用笔，先来领钥匙；用完，把钥匙还回去。领着钥匙的那位，叫 **owner（当前持有者）**；想用却没领到的，只能在门外排队等钥匙还回来，叫 **waiter（等待者）**。这把钥匙，就是**互斥锁 mutex**。

注意它和传送带（队列）的分工不一样：队列关心"**东西**从谁传到谁"，钥匙关心"这件**独占资源**眼下归谁"。所以读 mutex，眼睛别盯数据，盯两样：**钥匙在谁手里（owner）、谁在门外等（waiter）**。

### 9.2 优先级反转：一个不相干的人，拖垮了急件

钥匙的规矩听着天经地义，可它会捅出一个特别阴、又特别经典的娄子。看这么一幕（三个人：LOG 低优先级、COMM 高优先级，中间还杵着个跟笔毫不相干的中优先级活，就叫它 MID）：

1. LOG 领了钥匙，正低头写它那长长的流水账；
2. COMM 来了急件，也要用笔——可钥匙在 LOG 手里，**COMM 只能等**。到这儿都还合理，急件等一下持有者，认了；
3. 坏就坏在这时候：**MID 醒了**。它根本不用笔，但它比 LOG 急，**一上来就把 LOG 从工作台上挤了下去**，自顾自干它的活。

结果呢？LOG 被 MID 压着、迟迟写不完、**还不了钥匙**；COMM 在门外**跟着一起干等**。绕了一圈，一个跟笔八竿子打不着、优先级还没 COMM 高的 MID，硬生生把最急的 COMM 拖住了。这就是臭名昭著的**优先级反转**——

> **优先级反转：高优先级任务，被一个跟锁毫不相干的中优先级任务，间接拖慢了。** 根子在于：占着钥匙的是个低优先级的人，他一被中优先级插队，那把钥匙就迟迟还不回来。

工头的补救，叫**优先级继承**：**当 COMM 来等 LOG 手里的钥匙时，工头临时把 LOG 提到和 COMM 一样急。** 这么一来 MID 就再也挤不动 LOG 了，LOG 得以尽快写完、把钥匙还掉；LOG 一还钥匙，**立刻被恢复成原来的低优先级**。

> 继承**不是**让 COMM 绕过钥匙抢笔——资源边界一直都在。它只是**给那个占着钥匙的低优先级的人临时"提级"，催他赶紧用完归还**，别被不相干的人插队拖死。

### 9.3 demo：owner 被临时"提级"，还锁后复原

[`v10_mutex_inheritance`](code/v10_mutex_inheritance/demo.c) 把这条链原样跑了一遍，顺着 owner 读最顺：

```output
LOW_LOG takes mutex
MID_WORK is ready and could preempt LOW_LOG if no inheritance exists
HIGH_COMM waits for mutex owned by LOW_LOG
inherit: LOW_LOG priority 1 -> 3
LOW_LOG releases mutex, priority restores 3 -> 1
```

一行行对着看：LOW_LOG 领钥匙成为 owner；MID_WORK 醒了、**本可以把 LOW 挤下去**（`could preempt ... if no inheritance`——这行就是反转的隐患）；HIGH_COMM 来等钥匙，触发 `inherit`，把 LOW 从 1 临时提到 3——**MID（2）这下压不动 LOW 了**；LOW 用完 `releases`，优先级从 3 复原回 1。

![优先级反转与继承的全过程](img/fig-015.png)

最该盯的证据是 `HIGH_COMM waits for mutex owned by LOW_LOG`：**COMM 优先级最高，却照样越不过还没还钥匙的 LOG**——资源所有权，比优先级更硬。

### 9.4 回到源码：锁为什么住在 queue.c

第一次翻源码会有点错愕：**mutex 的代码，居然在 `queue.c` 里。** 别别扭——FreeRTOS 干脆复用了整套队列机制来实现锁和信号量（一把锁，就当成一个"容量为 1、里头装的不是数据而是所有权"的特殊队列）。只要抓住 owner 和等待链，文件名带不偏你：

| 环节 | 源码入口 | 干的活 |
| --- | --- | --- |
| 造一把钥匙 | [`xQueueCreateMutex()` `L647`](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L647) | 沿队列路径创建 mutex（[`prvInitialiseMutex` `L617`](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L617) 初始化 owner 字段） |
| 领钥匙（可能没领到） | [`xQueueSemaphoreTake()` `L1659`](../../reference/rtos_src/FreeRTOS-Kernel/queue.c#L1659) | 有 owner 时，把自己挂进等待链 |
| 临时提级 | [`xTaskPriorityInherit()` `L6650`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L6650) | 把 owner 提到等待者的优先级 |
| 还钥匙后复原 | [`xTaskPriorityDisinherit()` `L6753`](../../reference/rtos_src/FreeRTOS-Kernel/tasks.c#L6753) | owner 释放锁后恢复原优先级 |

![优先级继承链](img/fig-036-mutex-priority-inheritance-chain.png)

排查上手就一句话：**高优先级任务卡住，先找 owner——谁持着锁、持了多久、锁里到底在干什么。** 把 owner、waiter、领锁 tick、还锁 tick、临时优先级这几样一起采，比一句"锁导致卡顿"精准得多。

最后是最要命、也最容易被误解的一条工程判断：

> **继承是止血，不是根治。** 它能省下"被中优先级插队"的那段时间，却**变不快慢串口本身，也替你缩不短持锁区**。真正该优化的，永远是**持锁区的大小**——别在握着钥匙时去干慢串口、擦 Flash、大段格式化。钥匙攥得越久，全班组等得越久。

到这儿，任务怎么跑、怎么等、怎么交接、怎么抢资源，都讲遍了。可我们一路都在用 `xTaskCreate`、`xQueueCreate`、`xSemaphoreCreateMutex` 凭空"变"出这些对象——**它们到底从哪块地长出来的？** 任务栈、队列缓冲、锁，桩桩都要占 RAM。下一节的 heap_4，就是管这块地的**地主**。
