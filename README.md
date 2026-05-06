# STM32F407 Learning Journey 

本项目是本人学习STM32边学边写的一点记录。顺手写了个教程。所谓六经注我我注六经，学习和教学本就是一体两面的。再加上ai时代，使得我不必拘泥于文字的细节，两三天就能写一章。教程记录着我在学习过程中思考到的一些问题和求证后的一些成果。

因此，这不单是stm32的教学，而更主要是我将stm32和我学校中学习到和实习中经历到的事情融合后的产物。难免会和普通的开发板例程有所不同。

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
│   ├── Chapter3_外设串讲_通信协议和ADC/
│   │   ├── Chapter3_外设串讲_通信协议和ADC.md
│   │   ├── code/
│   │   └── img/
│   ├── Chapter4_玩转总线_DMA和FSMC/
│   │   ├── Chapter4_玩转总线_DMA和FSMC.md
│   │   ├── code/                       # DMA_UART_TX + FSMC_SRAM_RW
│   │   ├── gen_images.py               # 插图生成脚本
│   │   └── img/
│   ├── Chapter5_C语言面向对象工程架构基础/
│   │   ├── Chapter5_C语言面向对象工程架构基础.md
│   │   ├── code/
│   │   └── img/
│   ├── Chapter6_手搓FreeRTOS_内核基石与静态任务管理/
│   │   ├── Chapter6_手搓FreeRTOS_内核基石与静态任务管理.md
│   │   ├── code/
│   │   └── img/
│   ├── Chapter7_手搓FreeRTOS_进程模型与定时/
│   │   ├── Chapter7_手搓FreeRTOS_进程模型与定时.md
│   │   ├── code/
│   │   └── img/
│   ├── Chapter8_手搓FreeRTOS_进程同步和信号量/
│   │   ├── Chapter8_手搓FreeRTOS_进程同步和信号量.md
│   │   ├── code/
│   │   └── img/
│
└── README.md                           # 本文件
```

## 📖 学习文档章节跳转

| 章节 | 标题 | 状态 |
|:---:|------|:---:|
| 1 | [寄存器点亮 LED](tutorials/Chapter1_寄存器点亮led/Chapter1_寄存器点亮led.md) | ✅ |
| 2 | [基础设施：中断系统、定时器和看门狗](tutorials/Chapter2_基础设施_中断系统,定时器和看门狗/Chapter2_基础设施_中断系统,定时器和看门狗.md) | ✅ |
| 3 | [外设串讲：通信协议（UART · SPI · I²C · CAN）](tutorials/Chapter3_外设串讲_通信协议和ADC/Chapter3_外设串讲_通信协议和ADC.md) | ✅ |
| 4 | [玩转总线：DMA 和 FSMC](tutorials/Chapter4_玩转总线_DMA和FSMC/Chapter4_玩转总线_DMA和FSMC.md) | ✅ |
| 5 | [C语言面向对象工程架构基础](tutorials/Chapter5_C语言面向对象工程架构基础/Chapter5_C语言面向对象工程架构基础.md) | 📝 |
| 6 | [手搓 FreeRTOS：内核基石与静态任务管理](tutorials/Chapter6_手搓FreeRTOS_内核基石与静态任务管理/Chapter6_手搓FreeRTOS_内核基石与静态任务管理.md) | 📝 |
| 7 | [手搓 FreeRTOS：进程模型与定时](tutorials/Chapter7_手搓FreeRTOS_进程模型与定时/Chapter7_手搓FreeRTOS_进程模型与定时.md) | 📝 |
| 8 | [手搓 FreeRTOS：进程同步和信号量](tutorials/Chapter8_手搓FreeRTOS_进程同步和信号量/Chapter8_手搓FreeRTOS_进程同步和信号量.md) | 📝 |

> ✅ 已完成 &nbsp;|&nbsp; 📝 编写中


