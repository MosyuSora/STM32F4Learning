# STM32F407 Learning Journey (霸天虎开发板)

本项目是老板（Luyu）学习 STM32F407ZGT6 的全过程记录。基于野火《零死角玩转 STM32 — 基于野火 F407 [霸天虎] 开发板》教程，通过“官方参考 + 核心拆解 + 自主重构”的方式，深入掌握嵌入式开发。

## 🎯 项目目的
- **深入底层**：从寄存器原理开始，透彻理解 ARM Cortex-M4 架构及外设控制。
- **实战导向**：重构官方例程，编写简洁、可读性强的学习代码（My Code）。
- **知识沉淀**：通过 Markdown 整理学习笔记，将千页教程浓缩为精华。

## 📂 目录结构
```text
.
├── reference/               # [参考资料区](./reference/README.md)
│   ├── pdf/                 # 野火官方教程（完整版 + 54个拆分章节版）
│   └── official_tutorial_code/ # 野火官方配套全部例程源码
├── tutorials/               # 学习文档：深入浅出的 Markdown 文档
├── code/                    # 学习文档工程代码（自主重构版）
└── README.md                # 本文件
```

## 📖 学习文档章节跳转

### 第一阶段：环境搭建与基础架构
- [ ] [01. 环境搭建与工具链配置 (Keil5 + DAP/串口下载)](./tutorials/01-Environment_Setup.md)
- [ ] [02. STM32 架构初识与存储器映射](./tutorials/02-STM32_Architecture.md)
- [ ] [03. 寄存器底层原理探究](./tutorials/03-Register_Principle.md)

### 第二阶段：通用外设实战 (寄存器/固件库)
- [ ] [04. GPIO 输出：寄存器点灯](./tutorials/04-GPIO_Output_REG.md)
- [ ] [05. 构建库函数雏形：自己写库](./tutorials/05-Build_Own_Library.md)
- [ ] [06. GPIO 输出：标准库点灯与流水灯](./tutorials/06-GPIO_Output_StdPeriph.md)
- [ ] [07. GPIO 输入：按键检测](./tutorials/07-GPIO_Input_Key.md)
- [ ] [08. 中断系统：NVIC 与 EXTI](./tutorials/08-Interrupt_EXTI.md)

### 第三阶段：核心系统与通讯
- [ ] [09. 时钟系统：RCC 配置详解](./tutorials/09-RCC_Clock_Config.md)
- [ ] [10. 系统定时器：SysTick](./tutorials/10-SysTick.md)
- [ ] [11. 串口通讯：USART/UART](./tutorials/11-USART_Communication.md)
- [ ] [12. 直接存储区访问：DMA](./tutorials/12-DMA_Transfer.md)

### 第四阶段：进阶总线与应用
- [ ] [13. I2C 通讯：读写 EEPROM](./tutorials/13-I2C_EEPROM)
- [ ] [14. SPI 通讯：读写串行 Flash](./tutorials/14-SPI_Flash)
- [ ] [15. 外部 SRAM 扩展：FSMC](./tutorials/15-FSMC_SRAM)
- [ ] [16. 液晶显示：LCD 驱动与绘图](./tutorials/16-LCD_Display)

---
*Generated and maintained by Phoebe (第一秘书 菲比) for Boss Luyu.*
