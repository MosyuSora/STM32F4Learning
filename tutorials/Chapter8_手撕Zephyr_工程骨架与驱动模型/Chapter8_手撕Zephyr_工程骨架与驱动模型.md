# Chapter 8 手撕 Zephyr：工程骨架与驱动模型

## 0 开场：从"请个帮工"到"进厂上工"

学到这里，你其实已经会不少了。Ch6 我们把 FreeRTOS 的内核从任务、调度、协作一路撕到了内存；Ch5 又把 C 语言的"面向对象"——ops 虚表、`container_of`、Interface/Driver/Platform 分层——讲透了。带着这两样去看 Zephyr，你会发现一件让人松一口气的事：

> **Zephyr 真正难的，从来不是它的内核——那些你 Ch6 已经会了。难的是它那套"工厂系统"。**

所以这一章不打算把内核再撕一遍（到 §8 我给你一张"换皮对照表"就够了）。我们要花力气拆的，是 FreeRTOS **根本没有**、而初学者一头撞进去就晕的那部分：Zephyr 的**构建系统、设备树、设备驱动模型**。想讲清它们为什么长这样，得先把 FreeRTOS 和 Zephyr 的**心智模型差别**摆正——这是本章第一块、也是最重要的一块地基。

### 0.1 FreeRTOS：请个帮工到你的作坊

回想 Ch6/Ch7 你是怎么用 FreeRTOS 的：**你的工程是主体。** `main()` 是你写的，构建脚本（Makefile 或 IDE 工程）是你的，板级初始化、时钟配置、`main` 里 `xTaskCreate` 谁先谁后——全听你的。FreeRTOS 只是你**拖进工程的几个 `.c` 文件**：你调它的 `xQueueCreate`、`vTaskDelay`，它老老实实服务你。

> **FreeRTOS 是一个库（library）。你是坊主，它是你请来的帮工——帮工听你指挥，进你的作坊、用你的规矩。**

这也是为什么 Ch7 讲"移植"时，核心就是把 `port.c`、`FreeRTOSConfig.h` 塞进**你的**工程、接上**你的**时钟和中断向量表。主体始终是你。

### 0.2 Zephyr：你去一家开好的工厂上工

Zephyr 把这个关系**整个翻了过来。**

你打开一个 Zephyr 工程会发现：`main()` 好像也是你写的，但它只是一个叫 `main` 的**普通线程**，在 Zephyr 早已跑完一大套初始化之后才被调起来；构建不是你的 Makefile，而是 Zephyr 的 **CMake + Kconfig + west** 一整套；板子长什么样、有哪些外设、GPIO 接在哪，不写在你的 `.c` 里，而是写在 Zephyr 规定的**设备树**（`.dts`）里；连你能不能用某个功能，都由一堆 **`CONFIG_*`** 开关决定。

> **Zephyr 是一套操作系统 + 工程体系（framework）。它才是主体——有自己的流水线（构建系统）、自己的仓储清单（设备树）、自己的厂规（Kconfig）、自己的调度中心（内核）。你写的应用，是挂在它体系里的一个 app。**

打个比方：**用 FreeRTOS 像请个帮工到你家作坊；用 Zephyr 像你去一家现代化工厂上工。** 工厂里流水线、仓库、规章、考勤系统全是现成的、也全是"必须按它来"的——你想干活，第一步不是撸起袖子写代码，而是**先学会用厂里这套系统**。很多人觉得 Zephyr"陡"，不是因为内核难，而是卡在了这套系统的门槛上。

### 0.3 这一章怎么走

既然难点在"工厂系统"，本章就顺着"**你入职这家工厂要依次搞懂什么**"来排：

1. **PART1 工程骨架**——先看懂这条流水线：一次 `west build` 到底发生了什么（§1）；厂规 Kconfig 怎么裁功能（§2，正接 Ch7 的"部署=配置裁切"）；仓储清单**设备树**怎么描述硬件（§3，正接 Ch5 §7.6 那句没展开的"设备树"）。
2. **PART2 设备与驱动模型**——这是 Zephyr 的心脏，也是 **Ch5 那套 OOP 的工业级答卷**：`struct device` 四件套（§4）、它怎么在编译期被"实例化"出来（§5）、一个真实驱动怎么用 ops 虚表 + `container_of`（§6）、开机时这些对象怎么分级初始化（§7）。
3. **PART3 内核**——到这儿你会心一笑：`k_thread`、`k_sem`、`k_mutex`……全是 Ch6 的老熟人换了个名字。给一张对照表 + 几个 Zephyr 独有点即可（§8）。
4. **PART4 收束**——Zephyr 和 FreeRTOS 的工程取舍，以及它和下一章 **Ch9 Linux** 的关系（§9）。

一句话定调这一章：**我们不是来重新学一个 RTOS 的，是来学会"在一套大型嵌入式操作系统体系里干活"的。** 这套本事，往上再走一步就是 Linux。

---

## PART1 工程骨架：先看懂这条流水线

## 1 一次 west build 到底发生了什么

> （待写）west 是什么、CMake + Kconfig + devicetree 三样如何在一次构建里汇流、生成 `zephyr.elf` 的关键中间产物（`.config`、`devicetree_generated.h`、`zephyr.dts`）。

## 2 Kconfig：厂规怎么裁功能

> （待写）`CONFIG_*` 从哪来、`prj.conf` 怎么写、`Kconfig` 符号与依赖、menuconfig；对照 Ch7"部署=配置裁切"。

## 3 设备树：仓储清单怎么描述硬件

> （待写）`.dts` + binding(`.yaml`) → `devicetree_generated.h`；`DT_*` / `DT_INST_*` 宏链；接 Ch5 §7.6。

## PART2 设备与驱动模型：Ch5 OOP 的工业答卷

## 4 struct device：config / api / state / data 四件套

> （待写）`struct device`（device.h）逐字段拆；`api` 就是 Ch5 §5.4 的 ops 虚表。

## 5 DEVICE_DT_DEFINE：对象怎么在编译期被实例化

> （待写）宏展开、iterable sections、静态实例化；对照 Ch6"对象=结构体+牌子"。

## 6 一个真实驱动：gpio_driver_api + container_of

> （待写）`gpio_driver_api` ops 表、驱动里 `CONTAINER_OF` 从 `struct device` 找回私有 data；接 Ch5 §6。

## 7 SYS_INIT 与分级启动：开机时对象怎么按级立起来

> （待写）init levels（PRE_KERNEL_1/2、POST_KERNEL、APPLICATION）、`SYS_INIT`、启动流程；main 只是最后被调起的一个线程。

## PART3 内核：Ch6 的老熟人换了个名字

## 8 内核映射：换皮对照表 + Zephyr 独有点

> （待写）k_thread/调度/k_sem/k_mutex/k_msgq/k_event ↔ Ch6 对照表；Zephyr 独有：协作+抢占优先级区间、可换调度器、SMP 自旋锁、meta-IRQ。深挖回指 Ch6。

## PART4 收束与落地

## 9 结语：工程取舍与通往 Linux

> （待写）Zephyr↔FreeRTOS 工程取舍（重但完整 vs 轻但自己搭）；devicetree/Kconfig 都借自 Linux 内核——自然引出 Ch9 Linux（真 MMU + 更完整的驱动模型）。
