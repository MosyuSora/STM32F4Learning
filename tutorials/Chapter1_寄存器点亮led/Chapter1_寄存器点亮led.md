# Chapter 1: 寄存器版点亮 LED

> 本章节汇总野火教程第 5、6、7、8 章内容，深入探讨 STM32 的硬件架构及寄存器底层操作。

## 1.1 初识 STM32 (Chapter 5)
- **内核与外设**：理解 Cortex-M4 内核与 ST 芯片外设的分工。
- **选型与引脚**：识别 STM32F407ZGT6 的引脚分布及功能。

## 1.2 什么是寄存器 (Chapter 6)
- **存储器映射 (Memory Map)**：
    - 外设基地址 (Peripherals Base Address)。
    - 总线偏移与寄存器偏移。
- **寄存器封装**：利用 C 语言结构体或宏定义实现地址到变量的映射。

## 1.3 新建工程—寄存器版 (Chapter 7)
- **工程结构**：启动文件 (startup_stm32f40xxx.s) 的作用。
- **编译与链接**：代码与数据在存储空间中的分布逻辑。

## 1.4 实战：使用寄存器点亮 LED (Chapter 8)
- **时钟控制**：RCC (Reset and Clock Control) 的 AHB1 外设时钟使能。
- **GPIO 配置**：
    - `GPIOx_MODER`: 设置输入/输出模式。
    - `GPIOx_OTYPER`: 设置推挽/开漏。
    - `GPIOx_OSPEEDR`: 设置引脚响应速度。
    - `GPIOx_PUPDR`: 设置上拉/下拉。
- **电平控制**：操作 `GPIOx_ODR` 或 `GPIOx_BSRR` 实现 LED 的亮灭。

## 1.5 个人思考与总结
- （老板，在此处记录您的感悟）

---
*Created by Phoebe.*
