# Reference - STM32F407 官方资料库

本目录存放野火官方提供的原始学习资料。代码文件夹编号（如 `11`, `12`, `21`）对应教程中的 **“篇-章”** 逻辑。

## 📑 教程与源码全景索引 (Comprehensive Index)

### 🟢 第一部分：基础入门篇 (环境搭建与底层原理)
这部分主要讲解开发环境的准备以及 STM32 的硬件基础、寄存器本质，大部分章节为纯理论或环境配置，无需独立代码工程。

| 章节 (PDF) | 标题内容 | 官方源码路径 (Official Code) | 备注 |
| :--- | :--- | :--- | :--- |
| Chapter 01 | [如何使用本书](./pdf/chapters/Chapter_01_如何使用本书.pdf) | 无需代码 | 学习方法指导 |
| Chapter 02 | [如何安装 KEIL5](./pdf/chapters/Chapter_02_如何安装_KEIL5.pdf) | 无需代码 | 环境搭建 |
| Chapter 03 | [如何用 DAP 仿真器下载程序](./pdf/chapters/Chapter_03_如何用_DAP_仿真器下载程序.pdf) | 无需代码 | 硬件连接与配置 |
| Chapter 04 | [如何用串口下载程序](./pdf/chapters/Chapter_04_如何用串口下载程序.pdf) | 无需代码 | ISP 一键下载原理 |
| Chapter 05 | [初识 STM32](./pdf/chapters/Chapter_05_初识_STM32.pdf) | 无需代码 | 芯片选型与背景 |
| Chapter 06 | [什么是寄存器](./pdf/chapters/Chapter_06_什么是寄存器.pdf) | 无需代码 | **核心理论**：存储器映射 |
| Chapter 07 | [新建工程—寄存器版](./pdf/chapters/Chapter_07_新建工程—寄存器版.pdf) | [7-新建工程-寄存器版本](./official_tutorial_code/7-新建工程-寄存器版本) | 手动建立寄存器工程 |
| Chapter 08 | [使用寄存器点亮 LED 灯](./pdf/chapters/Chapter_08_使用寄存器点亮_LED_灯.pdf) | [8-使用寄存器点亮LED灯](./official_tutorial_code/8-使用寄存器点亮LED灯) | 第一个底层实战 |
| Chapter 09 | [自己写库—构建库函数雏形](./pdf/chapters/Chapter_09_自己写库—构建库函数雏形.pdf) | [9-自己写库—构建库函数雏形](./official_tutorial_code/9-自己写库—构建库函数雏形) | 揭秘固件库本质 |

### 🔵 第二部分：固件库开发篇 (核心外设使用)
从第 11 章（即 1-1）开始，正式进入 ST 标准固件库的学习阶段。

| 章节 (PDF) | 标题内容 | 官方源码路径 (Official Code) | 备注 |
| :--- | :--- | :--- | :--- |
| Chapter 10 | [初识 STM32 固件库](./pdf/chapters/Chapter_10_初识_STM32_固件库.pdf) | 无需代码 | 固件库文件结构分析 |
| Chapter 11 | [新建工程—库函数版](./pdf/chapters/Chapter_11_新建工程—库函数版.pdf) | [11-新建工程-固件库版本](./official_tutorial_code/11-新建工程-固件库版本) | 1-1：标准化工程模板 |
| Chapter 12 | [GPIO 输出—固件库点灯](./pdf/chapters/Chapter_12_GPIO_输出—使用固件库点灯.pdf) | [12-GPIO输出—点亮LED灯](./official_tutorial_code/12-GPIO输出—使用固件库点亮LED灯) | 1-2：包含 LED 与蜂鸣器例程 |
| Chapter 13 | [GPIO 输入—按键检测](./pdf/chapters/Chapter_13_GPIO_输入—按键检测.pdf) | [13-GPIO输入—按键检测](./official_tutorial_code/13-GPIO输入—按键检测) | 1-3：基础输入实验 |
| Chapter 14 | [GPIO—位带操作](./pdf/chapters/Chapter_14_GPIO—位带操作.pdf) | [14-位带操作](./official_tutorial_code/14-位带操作) | 1-4：51 风格的 IO 控制 |
| Chapter 15 | [启动文件详解](./pdf/chapters/Chapter_15_启动文件详解.pdf) | [15-启动文件详解](./official_tutorial_code/15-启动文件详解) | 1-5：汇编层启动流程分析 |
| Chapter 16 | [RCC—时钟配置](./pdf/chapters/Chapter_16_RCC—使用_HSE_HSI_配置时钟.pdf) | [16-RCC—时钟配置](./official_tutorial_code/16-RCC—时钟配置（使用HSE或者HSI）) | 1-6：系统心脏的配置 |
| Chapter 17 | [STM32 中断应用概览](./pdf/chapters/Chapter_17_STM32_中断应用概览.pdf) | 无需代码 | 中断管理理论 (NVIC) |
| Chapter 18 | [EXTI—外部中断](./pdf/chapters/Chapter_18_EXTI—外部中断_事件控制.pdf) | [18-EXTI—外部中断](./official_tutorial_code/18-EXTI—外部中断) | 1-8：硬件触发中断 |
| Chapter 19 | [SysTick—系统定时器](./pdf/chapters/Chapter_19_SysTick—系统定时器.pdf) | [19-SysTick—系统定时器](./official_tutorial_code/19-SysTick—系统定时器) | 1-9：精确延时与节拍 |

### 🔴 第三部分：高级外设实战篇 (通讯与系统应用)
涵盖 2x 至 5x 章节，对应教程的后续篇章（如第 2 篇通讯、第 3 篇存储等）。

| 章节 (PDF) | 标题内容 | 官方源码路径 (Official Code) | 备注 |
| :--- | :--- | :--- | :--- |
| Chapter 21 | [USART—串口通讯](./pdf/chapters/Chapter_21_USART—串口通讯.pdf) | [21-USART—串口通信](./official_tutorial_code/21-USART—串口通信) | 2-1：异步通讯实战 |
| Chapter 22 | [DMA—直接存储区访问](./pdf/chapters/Chapter_22_DMA—直接存储区访问.pdf) | [22-DMA—直接存储区访问](./official_tutorial_code/22-DMA—直接存储区访问) | 2-2：数据搬运“临时工” |
| Chapter 24 | [I2C—读写 EEPROM](./pdf/chapters/Chapter_24_I2C—读写_EEPROM.pdf) | [24-I2C-读写EEPROM](./official_tutorial_code/24-I2C-读写EEPROM) | 2-4：同步串行通讯 |
| Chapter 25 | [SPI—读写串行 FLASH](./pdf/chapters/Chapter_25_SPI—读写_串行_FLASH.pdf) | [25-SPI—读写串行FLASH](./official_tutorial_code/25-SPI—读写串行FLASH（W25Q128）) | 2-5：高速串行通讯 |
| Chapter 26 | [FatFs 文件系统](./pdf/chapters/Chapter_26_串行_FLASH_文件系统_FatFs.pdf) | [26-串行FLASH文件系统FatFs](./official_tutorial_code/26-串行FLASH文件系统FatFs) | 文件系统移植 |
| Chapter 27 | [FSMC—扩展外部 SRAM](./pdf/chapters/Chapter_27_FSMC—扩展外部_SRAM.pdf) | [27-FSMC—扩展外部SRAM](./official_tutorial_code/27-FSMC—扩展外部SRAM) | 内存扩展技术 |
| Chapter 28 | [LCD—液晶显示](./pdf/chapters/Chapter_28_LCD—液晶显示.pdf) | [28-液晶显示](./official_tutorial_code/28-液晶显示) | 屏幕驱动基础 |
| Chapter 29 | [LCD—中英文显示](./pdf/chapters/Chapter_29_LCD—液晶显示中英文.pdf) | [29-液晶显示中英文](./official_tutorial_code/29-液晶显示中英文) | 字符显示与字库 |
| Chapter 30 | [电容触摸屏](./pdf/chapters/Chapter_30_电容触摸屏—触摸画板.pdf) | [30-电容触摸屏—触摸画板](./official_tutorial_code/30-电容触摸屏—触摸画板) | 交互界面开发 |
| Chapter 31 | [ADC—电压采集](./pdf/chapters/Chapter_31_ADC—电压采集.pdf) | [31-ADC电压采集](./official_tutorial_code/31-ADC电压采集) | 模拟信号转换 |
| Chapter 32 | [TIM—基本定时器](./pdf/chapters/Chapter_32_TIM—基本定时器.pdf) | [32-TIM—基本定时器定时](./official_tutorial_code/32-TIM—基本定时器定时) | 定时任务基础 |
| Chapter 33 | [TIM—高级定时器](./pdf/chapters/Chapter_33_TIM—高级定时器.pdf) | [33-TIM—高级定时器](./official_tutorial_code/33-TIM—高级定时器) | 包含 PWM 输出等 |
| Chapter 37 | [SDIO—SD 卡读写](./pdf/chapters/Chapter_37_SDIO—SD_卡读写测试.pdf) | [37-SDIO—SD卡读写测试](./official_tutorial_code/37-SDIO—SD卡读写测试) | 大容量存储应用 |
| Chapter 43 | [ETH—以太网通信](./pdf/chapters/Chapter_43_ETH—Lwip_以太网通信.pdf) | [43-ETH—LWIP以太网](./official_tutorial_code/43-ETH—LWIP以太网) | 联网实战 |
| Chapter 44 | [CAN—通讯实验](./pdf/chapters/Chapter_44_CAN—通讯实验.pdf) | [44-CAN通信实验](./official_tutorial_code/44-CAN通信实验) | 工业总线实战 |
| Chapter 54 | [DCMI—OV5640 摄像头](./pdf/chapters/Chapter_54_DCMI—OV5640_摄像头.pdf) | [54-DCMI—OV5640摄像头](./official_tutorial_code/54-DCMI—OV5640摄像头) | 视频采集实战 |

---
*Index Refined by Phoebe (第一秘书 菲比) for Luyu Boss.*
