# STM32F407 Learning Journey (霸天虎开发板)

本项目是学习 STM32F407ZGT6 的全过程记录。基于野火《零死角玩转 STM32 — 基于野火 F407 [霸天虎] 开发板》教程，通过“官方参考 + 核心拆解 + 自主重构”的方式，深入掌握嵌入式开发。

## 🎯 项目目的
- **深入底层**：从寄存器原理开始，透彻理解 ARM Cortex-M4 架构及外设控制。
- **实战导向**：重构官方例程，编写简洁、可读性强的学习代码（My Code）。
- **知识沉淀**：通过 Markdown 整理学习笔记，将千页教程浓缩为精华。

## 📂 目录结构

```
.
├── reference/                          # 参考资料区（详见 reference/README.md）
│   ├── pdf/                            # 野火官方教程（完整版 + 54 个拆分章节版）
│   └── official_tutorial_code/         # 野火官方配套全部例程源码
│
├── tutorials/                          # 学习文档 & 配套代码
│   ├── Chapter1_寄存器点亮led/
│   │   ├── Chapter1_寄存器点亮led.md
│   │   ├── code/                       # 工程代码
│   │   └── img/                        # 文档插图
│   ├── Chapter2_基础设施_中断系统,定时器和看门狗/
│   │   ├── Chapter2_基础设施_中断系统,定时器和看门狗.md
│   │   ├── code/                       # 工程代码（noWDG / WDG 两个变体）
│   │   └── img/
│   ├── Chapter3 ~ Chapter9 …           # 后续章节（持续更新中）
│
└── README.md                           # 本文件
```

## 📖 学习文档章节跳转

| 章节 | 标题 | 状态 |
|:---:|------|:---:|
| 1 | [寄存器点亮 LED](tutorials/Chapter1_寄存器点亮led/Chapter1_寄存器点亮led.md) | ✅ |
| 2 | [基础设施：中断系统、定时器和看门狗](tutorials/Chapter2_基础设施_中断系统,定时器和看门狗/Chapter2_基础设施_中断系统,定时器和看门狗.md) | ✅ |
| 3 | [外设串讲：三大低速协议和 ADC](tutorials/Chapter3_外设串讲_三大低速协议和ADC/Chapter3_外设串讲_三大低速协议和ADC.md) | 📝 |
| 4 | [玩转总线：DMA 和 FSMC](tutorials/Chapter4_玩转总线_DMA和FSMC/Chapter4_玩转总线_DMA和FSMC.md) | 📝 |
| 5 | [手搓 FreeRTOS：内核基石与静态任务管理](tutorials/Chapter5_手搓FreeRTOS_内核基石与静态任务管理/Chapter5_手搓FreeRTOS_内核基石与静态任务管理.md) | 📝 |
| 6 | [手搓 FreeRTOS：进程模型与定时](tutorials/Chapter6_手搓FreeRTOS_进程模型与定时/Chapter6_手搓FreeRTOS_进程模型与定时.md) | 📝 |
| 7 | [手搓 FreeRTOS：进程同步和信号量](tutorials/Chapter7_手搓FreeRTOS_进程同步和信号量/Chapter7_手搓FreeRTOS_进程同步和信号量.md) | 📝 |
| 8 | [硬实时开发基础：状态机和前后台系统](tutorials/Chapter8_硬实时开发基础_状态机和前后台系统/Chapter8_硬实时开发基础_状态机和前后台系统.md) | 📝 |
| 9 | [硬实时开发进阶：计算模型和同步数据流](tutorials/Chapter9_硬实时开发进阶_计算模型和同步数据流/Chapter9_硬实时开发进阶_计算模型和同步数据流.md) | 📝 |

> ✅ 已完成 &nbsp;|&nbsp; 📝 编写中


