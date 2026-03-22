/**
  ******************************************************************************
  * @file      startup_stm32f407xx.s
  * @author    MCD Application Team
  * @brief     STM32F407xx Devices vector table for GCC based toolchains.
  *            本模块完成以下工作：
  *                - 设置初始栈指针 SP
  *                - 设置初始 PC == Reset_Handler（复位后的入口函数）
  *                - 将中断向量表各入口填入对应的异常/中断处理函数地址
  *                - 最终跳转到 C 库入口，再调用用户的 main() 函数
  *            复位后 Cortex-M4 处于线程模式(Thread mode)，
  *            特权级(Privileged)，使用主栈指针(MSP)。
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* =========================
 * GCC 汇编阅读小词典（先看这个再往下读）
 *
 * .syntax unified : 使用 ARM/Thumb 统一汇编语法
 * .cpu cortex-m4  : 指定目标 CPU 为 Cortex-M4
 * .fpu softvfp    : 使用软件浮点（不生成硬件浮点指令）
 * .thumb          : 生成 Thumb 指令集代码
 *
 * .global  : 把符号公开给链接器（类似 Keil 的 EXPORT）
 * .weak    : 声明弱符号（可被其他文件的同名强符号覆盖）
 * .word    : 定义一个 32 位数据（类似 Keil 的 DCD）
 * .section : 定义一个段（类似 Keil 的 AREA）
 * .type    : 声明符号类型（%function=函数，%object=数据）
 * .size    : 告诉链接器符号占多少字节
 *
 * .thumb_set A, B : 将符号 A 设为 B 的别名（Thumb 模式）
 *                   效果：如果没人定义 A，A 就等于 B
 *                   配合 .weak 使用 = Keil 的 EXPORT [WEAK]
 *
 * ldr Rn, =xxx   : 把常量或地址装入寄存器 Rn
 * bl  xxx        : 带链接跳转（保存返回地址到 LR，类似函数调用）
 * bx  lr         : 跳转到 LR 保存的地址（函数返回）
 * b   .          : 跳到自己，形成死循环
 * bcc label      : 如果无进位（小于）则跳转
 *
 * KEEP(*(.isr_vector)) : 链接脚本指令，保证向量表不被优化丢弃
 * ========================= */

  .syntax unified       /* 使用统一汇编语法 */
  .cpu cortex-m4        /* 目标 CPU: Cortex-M4 */
  .fpu softvfp          /* 软件浮点 */
  .thumb                /* 使用 Thumb 指令集 */

/* 导出向量表和默认中断处理函数符号，供链接器使用 */
.global  g_pfnVectors
.global  Default_Handler

/* =========================
 * 以下 .word 声明了 5 个链接脚本符号的引用。
 * 它们的值由链接脚本（.ld 文件）在链接阶段确定。
 * 启动代码通过这些符号完成 .data 拷贝和 .bss 清零。
 *
 * 注意：GCC 版和 Keil 版的重要区别——
 *   Keil 版：栈/堆空间直接在汇编文件里用 SPACE 分配
 *   GCC 版：栈/堆由链接脚本（.ld）分配，这里只引用符号
 * ========================= */
.word  _sidata           /* .data 段初始值在 FLASH 中的起始地址（源地址） */
.word  _sdata            /* .data 段在 RAM 中的起始地址（目的地址） */
.word  _edata            /* .data 段在 RAM 中的结束地址 */
.word  _sbss             /* .bss 段在 RAM 中的起始地址 */
.word  _ebss             /* .bss 段在 RAM 中的结束地址 */

/* =========================
 * 复位处理函数（启动主流程）
 * 芯片上电/复位后执行的第一个函数。
 * 完成以下工作：
 *   1) 设置栈指针 SP
 *   2) 调用 SystemInit 配置时钟和向量表偏移
 *   3) 将 .data 段的初始值从 FLASH 拷贝到 RAM
 *   4) 将 .bss 段清零（未初始化的全局/静态变量）
 *   5) 调用 C/C++ 全局构造函数
 *   6) 跳转到 main()
 *
 * 注意与 Keil 版的区别：
 *   Keil 版只做 步骤1→2→跳到 __main，由 C 运行库完成 3/4/5
 *   GCC  版在这里手动完成 3/4/5，因为 GCC 没有 Keil 的 __main 机制
 * ========================= */

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
  ldr   sp, =_estack     /* 步骤1: 设置栈指针到 RAM 末尾（_estack 由链接脚本定义） */

  /* 步骤2: 调用 SystemInit（在 system_stm32f4xx.c 中实现）
   *        配置系统时钟、设置向量表偏移量(VTOR)等 */
  bl  SystemInit

  /* =========================
   * 步骤3: 将 .data 段初始值从 FLASH 拷贝到 RAM
   *
   * 等价的 C 代码：
   *   src  = _sidata;   // FLASH 中的初始值起始地址
   *   dst  = _sdata;    // RAM 中 .data 段起始地址
   *   end  = _edata;    // RAM 中 .data 段结束地址
   *   while (dst < end) { *dst++ = *src++; }
   *
   * 寄存器分配：
   *   r0 = _sdata（RAM 目标起始地址）
   *   r1 = _edata（RAM 目标结束地址）
   *   r2 = _sidata（FLASH 源起始地址）
   *   r3 = 偏移量（从 0 开始，每次 +4 字节）
   * ========================= */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit        /* 先跳到循环条件判断 */

CopyDataInit:
  ldr r4, [r2, r3]          /* r4 = *(FLASH源地址 + 偏移) */
  str r4, [r0, r3]          /* *(RAM目标地址 + 偏移) = r4 */
  adds r3, r3, #4           /* 偏移 += 4（一次拷贝 4 字节 = 1 个字） */

LoopCopyDataInit:
  adds r4, r0, r3           /* r4 = RAM目标起始 + 当前偏移 = 当前写入地址 */
  cmp r4, r1                /* 比较当前地址和 _edata */
  bcc CopyDataInit          /* 如果还没到末尾，继续拷贝 */

  /* =========================
   * 步骤4: 将 .bss 段清零
   *
   * 等价的 C 代码：
   *   dst = _sbss;
   *   end = _ebss;
   *   while (dst < end) { *dst++ = 0; }
   *
   * 寄存器分配：
   *   r2 = 当前地址（从 _sbss 开始）
   *   r4 = _ebss（结束地址）
   *   r3 = 0（用于清零）
   * ========================= */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss         /* 先跳到循环条件判断 */

FillZerobss:
  str  r3, [r2]             /* *r2 = 0（当前地址清零） */
  adds r2, r2, #4           /* r2 += 4（移到下一个字） */

LoopFillZerobss:
  cmp r2, r4                /* 比较当前地址和 _ebss */
  bcc FillZerobss           /* 如果还没到末尾，继续清零 */

  /* 步骤5: 调用 C/C++ 全局构造函数（如 C++ 全局对象的构造）
   *        Keil 版这一步由 __main 内部完成 */
    bl __libc_init_array

  /* 步骤6: 跳转到用户的 main() 函数 */
  bl  main

  /* 如果 main() 返回，就回到调用者（实际不应该返回） */
  bx  lr
.size  Reset_Handler, .-Reset_Handler

/* =========================
 * 默认异常/中断处理函数
 * 当处理器收到一个未被用户实现的中断时，最终会跳到这里。
 * 死循环设计：方便用调试器暂停后定位是哪个中断触发了问题。
 *
 * 与 Keil 版对应：Keil 版写 B . （跳到自己），效果完全一样。
 * ========================= */
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop          /* 死循环，等价于 Keil 的 B . */
  .size  Default_Handler, .-Default_Handler
/* =========================
 * 中断向量表
 * 芯片复位后，CPU 从 FLASH 起始地址（0x0800_0000）读取：
 *   第 0 个字 → 初始栈指针 MSP
 *   第 1 个字 → 复位入口地址（Reset_Handler）
 *   后续各字 → 各异常/中断的处理函数地址
 *
 * .section .isr_vector 会被链接脚本放在 FLASH 最前面。
 *   链接脚本中 KEEP(*(.isr_vector)) 保证此段不被优化丢弃。
 *
 * .word 相当于 Keil 的 DCD（Define Constant Data，32 位）。
 *
 * 与 Keil 版的区别：
 *   Keil 版初始栈顶 = __initial_sp（在汇编文件里用 SPACE 分配）
 *   GCC  版初始栈顶 = _estack（由链接脚本计算 = RAM 末尾地址）
 * ========================= */
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object


g_pfnVectors:
  .word  _estack                              /* 初始主栈指针 MSP（栈顶地址） */
  .word  Reset_Handler                        /* 复位后执行的第一个函数 */
  .word  NMI_Handler                          /* NMI 不可屏蔽中断 */
  .word  HardFault_Handler                    /* 硬件错误（常见于访问异常） */
  .word  MemManage_Handler                    /* 存储器管理错误（MPU 相关） */
  .word  BusFault_Handler                     /* 总线访问错误 */
  .word  UsageFault_Handler                   /* 指令/状态使用错误 */
  .word  0                                    /* 保留 */
  .word  0                                    /* 保留 */
  .word  0                                    /* 保留 */
  .word  0                                    /* 保留 */
  .word  SVC_Handler                          /* SVC 系统服务调用 */
  .word  DebugMon_Handler                     /* 调试监控异常 */
  .word  0                                    /* 保留 */
  .word  PendSV_Handler                       /* PendSV（常用于 RTOS 任务切换） */
  .word  SysTick_Handler                      /* SysTick 系统滴答定时器 */

  /* 外部中断/外设中断入口（名称和芯片手册保持一致） */
  .word     WWDG_IRQHandler                   /* Window WatchDog              */                                        
  .word     PVD_IRQHandler                    /* PVD through EXTI Line detection */                        
  .word     TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */            
  .word     RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */                      
  .word     FLASH_IRQHandler                  /* FLASH                        */                                          
  .word     RCC_IRQHandler                    /* RCC                          */                                            
  .word     EXTI0_IRQHandler                  /* EXTI Line0                   */                        
  .word     EXTI1_IRQHandler                  /* EXTI Line1                   */                          
  .word     EXTI2_IRQHandler                  /* EXTI Line2                   */                          
  .word     EXTI3_IRQHandler                  /* EXTI Line3                   */                          
  .word     EXTI4_IRQHandler                  /* EXTI Line4                   */                          
  .word     DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                */                  
  .word     DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                */                   
  .word     DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                */                   
  .word     DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                */                   
  .word     DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                */                   
  .word     DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                */                   
  .word     DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                */                   
  .word     ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s         */                   
  .word     CAN1_TX_IRQHandler                /* CAN1 TX                      */                         
  .word     CAN1_RX0_IRQHandler               /* CAN1 RX0                     */                          
  .word     CAN1_RX1_IRQHandler               /* CAN1 RX1                     */                          
  .word     CAN1_SCE_IRQHandler               /* CAN1 SCE                     */                          
  .word     EXTI9_5_IRQHandler                /* External Line[9:5]s          */                          
  .word     TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9          */         
  .word     TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10        */         
  .word     TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11 */
  .word     TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */                          
  .word     TIM2_IRQHandler                   /* TIM2                         */                   
  .word     TIM3_IRQHandler                   /* TIM3                         */                   
  .word     TIM4_IRQHandler                   /* TIM4                         */                   
  .word     I2C1_EV_IRQHandler                /* I2C1 Event                   */                          
  .word     I2C1_ER_IRQHandler                /* I2C1 Error                   */                          
  .word     I2C2_EV_IRQHandler                /* I2C2 Event                   */                          
  .word     I2C2_ER_IRQHandler                /* I2C2 Error                   */                            
  .word     SPI1_IRQHandler                   /* SPI1                         */                   
  .word     SPI2_IRQHandler                   /* SPI2                         */                   
  .word     USART1_IRQHandler                 /* USART1                       */                   
  .word     USART2_IRQHandler                 /* USART2                       */                   
  .word     USART3_IRQHandler                 /* USART3                       */                   
  .word     EXTI15_10_IRQHandler              /* External Line[15:10]s        */                          
  .word     RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line */                 
  .word     OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line */                       
  .word     TIM8_BRK_TIM12_IRQHandler         /* TIM8 Break and TIM12         */         
  .word     TIM8_UP_TIM13_IRQHandler          /* TIM8 Update and TIM13        */         
  .word     TIM8_TRG_COM_TIM14_IRQHandler     /* TIM8 Trigger and Commutation and TIM14 */
  .word     TIM8_CC_IRQHandler                /* TIM8 Capture Compare         */                          
  .word     DMA1_Stream7_IRQHandler           /* DMA1 Stream7                 */                          
  .word     FSMC_IRQHandler                   /* FSMC                         */                   
  .word     SDIO_IRQHandler                   /* SDIO                         */                   
  .word     TIM5_IRQHandler                   /* TIM5                         */                   
  .word     SPI3_IRQHandler                   /* SPI3                         */                   
  .word     UART4_IRQHandler                  /* UART4                        */                   
  .word     UART5_IRQHandler                  /* UART5                        */                   
  .word     TIM6_DAC_IRQHandler               /* TIM6 and DAC1&2 underrun errors */                   
  .word     TIM7_IRQHandler                   /* TIM7                         */
  .word     DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                */                   
  .word     DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                */                   
  .word     DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                */                   
  .word     DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                */                   
  .word     DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                */                   
  .word     ETH_IRQHandler                    /* Ethernet                     */                   
  .word     ETH_WKUP_IRQHandler               /* Ethernet Wakeup through EXTI line */                     
  .word     CAN2_TX_IRQHandler                /* CAN2 TX                      */                          
  .word     CAN2_RX0_IRQHandler               /* CAN2 RX0                     */                          
  .word     CAN2_RX1_IRQHandler               /* CAN2 RX1                     */                          
  .word     CAN2_SCE_IRQHandler               /* CAN2 SCE                     */                          
  .word     OTG_FS_IRQHandler                 /* USB OTG FS                   */                   
  .word     DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                */                   
  .word     DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                */                   
  .word     DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                */                   
  .word     USART6_IRQHandler                 /* USART6                       */                    
  .word     I2C3_EV_IRQHandler                /* I2C3 event                   */                          
  .word     I2C3_ER_IRQHandler                /* I2C3 error                   */                          
  .word     OTG_HS_EP1_OUT_IRQHandler         /* USB OTG HS End Point 1 Out   */                   
  .word     OTG_HS_EP1_IN_IRQHandler          /* USB OTG HS End Point 1 In    */                   
  .word     OTG_HS_WKUP_IRQHandler            /* USB OTG HS Wakeup through EXTI */                         
  .word     OTG_HS_IRQHandler                 /* USB OTG HS                   */                   
  .word     DCMI_IRQHandler                   /* DCMI                         */                   
  .word     0                                 /* CRYP crypto                  */                   
  .word     HASH_RNG_IRQHandler               /* Hash and Rng                 */
  .word     FPU_IRQHandler                    /* FPU                          */
                         
                         

  .size  g_pfnVectors, .-g_pfnVectors

/* =========================
 * 为每个异常/中断处理函数提供弱别名(weak alias)，指向 Default_Handler。
 *
 * 机制说明：
 *   .weak      XXX_IRQHandler           → 声明为弱符号
 *   .thumb_set XXX_IRQHandler, Default_Handler  → 将 XXX_IRQHandler 设为
 *                                                  Default_Handler 的别名
 *
 * 效果：
 *   如果用户没有在 C 文件中实现 XXX_IRQHandler，则使用这里的弱定义
 *   （即跳到 Default_Handler 死循环）。
 *   如果用户在 C 文件中写了同名函数（如 void USART1_IRQHandler(void){...}），
 *   链接器会自动用用户的"强定义"覆盖这里的弱版本。
 *
 * 与 Keil 版的对应关系：
 *   Keil:  EXPORT  XXX_IRQHandler  [WEAK]
 *   GCC:   .weak   XXX_IRQHandler
 *          .thumb_set XXX_IRQHandler, Default_Handler
 * ========================= */
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler
  
   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler
  
   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler
  
   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler              
  
   .weak      WWDG_IRQHandler                   
   .thumb_set WWDG_IRQHandler,Default_Handler      
                  
   .weak      PVD_IRQHandler      
   .thumb_set PVD_IRQHandler,Default_Handler
               
   .weak      TAMP_STAMP_IRQHandler            
   .thumb_set TAMP_STAMP_IRQHandler,Default_Handler
            
   .weak      RTC_WKUP_IRQHandler                  
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler
            
   .weak      FLASH_IRQHandler         
   .thumb_set FLASH_IRQHandler,Default_Handler
                  
   .weak      RCC_IRQHandler      
   .thumb_set RCC_IRQHandler,Default_Handler
                  
   .weak      EXTI0_IRQHandler         
   .thumb_set EXTI0_IRQHandler,Default_Handler
                  
   .weak      EXTI1_IRQHandler         
   .thumb_set EXTI1_IRQHandler,Default_Handler
                     
   .weak      EXTI2_IRQHandler         
   .thumb_set EXTI2_IRQHandler,Default_Handler 
                 
   .weak      EXTI3_IRQHandler         
   .thumb_set EXTI3_IRQHandler,Default_Handler
                        
   .weak      EXTI4_IRQHandler         
   .thumb_set EXTI4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream0_IRQHandler               
   .thumb_set DMA1_Stream0_IRQHandler,Default_Handler
         
   .weak      DMA1_Stream1_IRQHandler               
   .thumb_set DMA1_Stream1_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream2_IRQHandler               
   .thumb_set DMA1_Stream2_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream3_IRQHandler               
   .thumb_set DMA1_Stream3_IRQHandler,Default_Handler 
                 
   .weak      DMA1_Stream4_IRQHandler              
   .thumb_set DMA1_Stream4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream5_IRQHandler               
   .thumb_set DMA1_Stream5_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream6_IRQHandler               
   .thumb_set DMA1_Stream6_IRQHandler,Default_Handler
                  
   .weak      ADC_IRQHandler      
   .thumb_set ADC_IRQHandler,Default_Handler
               
   .weak      CAN1_TX_IRQHandler   
   .thumb_set CAN1_TX_IRQHandler,Default_Handler
            
   .weak      CAN1_RX0_IRQHandler                  
   .thumb_set CAN1_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN1_RX1_IRQHandler                  
   .thumb_set CAN1_RX1_IRQHandler,Default_Handler
            
   .weak      CAN1_SCE_IRQHandler                  
   .thumb_set CAN1_SCE_IRQHandler,Default_Handler
            
   .weak      EXTI9_5_IRQHandler   
   .thumb_set EXTI9_5_IRQHandler,Default_Handler
            
   .weak      TIM1_BRK_TIM9_IRQHandler            
   .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler
            
   .weak      TIM1_UP_TIM10_IRQHandler            
   .thumb_set TIM1_UP_TIM10_IRQHandler,Default_Handler
      
   .weak      TIM1_TRG_COM_TIM11_IRQHandler      
   .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler
      
   .weak      TIM1_CC_IRQHandler   
   .thumb_set TIM1_CC_IRQHandler,Default_Handler
                  
   .weak      TIM2_IRQHandler            
   .thumb_set TIM2_IRQHandler,Default_Handler
                  
   .weak      TIM3_IRQHandler            
   .thumb_set TIM3_IRQHandler,Default_Handler
                  
   .weak      TIM4_IRQHandler            
   .thumb_set TIM4_IRQHandler,Default_Handler
                  
   .weak      I2C1_EV_IRQHandler   
   .thumb_set I2C1_EV_IRQHandler,Default_Handler
                     
   .weak      I2C1_ER_IRQHandler   
   .thumb_set I2C1_ER_IRQHandler,Default_Handler
                     
   .weak      I2C2_EV_IRQHandler   
   .thumb_set I2C2_EV_IRQHandler,Default_Handler
                  
   .weak      I2C2_ER_IRQHandler   
   .thumb_set I2C2_ER_IRQHandler,Default_Handler
                           
   .weak      SPI1_IRQHandler            
   .thumb_set SPI1_IRQHandler,Default_Handler
                        
   .weak      SPI2_IRQHandler            
   .thumb_set SPI2_IRQHandler,Default_Handler
                  
   .weak      USART1_IRQHandler      
   .thumb_set USART1_IRQHandler,Default_Handler
                     
   .weak      USART2_IRQHandler      
   .thumb_set USART2_IRQHandler,Default_Handler
                     
   .weak      USART3_IRQHandler      
   .thumb_set USART3_IRQHandler,Default_Handler
                  
   .weak      EXTI15_10_IRQHandler               
   .thumb_set EXTI15_10_IRQHandler,Default_Handler
               
   .weak      RTC_Alarm_IRQHandler               
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler
            
   .weak      OTG_FS_WKUP_IRQHandler         
   .thumb_set OTG_FS_WKUP_IRQHandler,Default_Handler
            
   .weak      TIM8_BRK_TIM12_IRQHandler         
   .thumb_set TIM8_BRK_TIM12_IRQHandler,Default_Handler
         
   .weak      TIM8_UP_TIM13_IRQHandler            
   .thumb_set TIM8_UP_TIM13_IRQHandler,Default_Handler
         
   .weak      TIM8_TRG_COM_TIM14_IRQHandler      
   .thumb_set TIM8_TRG_COM_TIM14_IRQHandler,Default_Handler
      
   .weak      TIM8_CC_IRQHandler   
   .thumb_set TIM8_CC_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream7_IRQHandler               
   .thumb_set DMA1_Stream7_IRQHandler,Default_Handler
                     
   .weak      FSMC_IRQHandler            
   .thumb_set FSMC_IRQHandler,Default_Handler
                     
   .weak      SDIO_IRQHandler            
   .thumb_set SDIO_IRQHandler,Default_Handler
                     
   .weak      TIM5_IRQHandler            
   .thumb_set TIM5_IRQHandler,Default_Handler
                     
   .weak      SPI3_IRQHandler            
   .thumb_set SPI3_IRQHandler,Default_Handler
                     
   .weak      UART4_IRQHandler         
   .thumb_set UART4_IRQHandler,Default_Handler
                  
   .weak      UART5_IRQHandler         
   .thumb_set UART5_IRQHandler,Default_Handler
                  
   .weak      TIM6_DAC_IRQHandler                  
   .thumb_set TIM6_DAC_IRQHandler,Default_Handler
               
   .weak      TIM7_IRQHandler            
   .thumb_set TIM7_IRQHandler,Default_Handler
         
   .weak      DMA2_Stream0_IRQHandler               
   .thumb_set DMA2_Stream0_IRQHandler,Default_Handler
               
   .weak      DMA2_Stream1_IRQHandler               
   .thumb_set DMA2_Stream1_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream2_IRQHandler               
   .thumb_set DMA2_Stream2_IRQHandler,Default_Handler
            
   .weak      DMA2_Stream3_IRQHandler               
   .thumb_set DMA2_Stream3_IRQHandler,Default_Handler
            
   .weak      DMA2_Stream4_IRQHandler               
   .thumb_set DMA2_Stream4_IRQHandler,Default_Handler
            
   .weak      ETH_IRQHandler      
   .thumb_set ETH_IRQHandler,Default_Handler
                  
   .weak      ETH_WKUP_IRQHandler                  
   .thumb_set ETH_WKUP_IRQHandler,Default_Handler
            
   .weak      CAN2_TX_IRQHandler   
   .thumb_set CAN2_TX_IRQHandler,Default_Handler
                           
   .weak      CAN2_RX0_IRQHandler                  
   .thumb_set CAN2_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN2_RX1_IRQHandler                  
   .thumb_set CAN2_RX1_IRQHandler,Default_Handler
                           
   .weak      CAN2_SCE_IRQHandler                  
   .thumb_set CAN2_SCE_IRQHandler,Default_Handler
                           
   .weak      OTG_FS_IRQHandler      
   .thumb_set OTG_FS_IRQHandler,Default_Handler
                     
   .weak      DMA2_Stream5_IRQHandler               
   .thumb_set DMA2_Stream5_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream6_IRQHandler               
   .thumb_set DMA2_Stream6_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream7_IRQHandler               
   .thumb_set DMA2_Stream7_IRQHandler,Default_Handler
                  
   .weak      USART6_IRQHandler      
   .thumb_set USART6_IRQHandler,Default_Handler
                        
   .weak      I2C3_EV_IRQHandler   
   .thumb_set I2C3_EV_IRQHandler,Default_Handler
                        
   .weak      I2C3_ER_IRQHandler   
   .thumb_set I2C3_ER_IRQHandler,Default_Handler
                        
   .weak      OTG_HS_EP1_OUT_IRQHandler         
   .thumb_set OTG_HS_EP1_OUT_IRQHandler,Default_Handler
               
   .weak      OTG_HS_EP1_IN_IRQHandler            
   .thumb_set OTG_HS_EP1_IN_IRQHandler,Default_Handler
               
   .weak      OTG_HS_WKUP_IRQHandler         
   .thumb_set OTG_HS_WKUP_IRQHandler,Default_Handler
            
   .weak      OTG_HS_IRQHandler      
   .thumb_set OTG_HS_IRQHandler,Default_Handler
                  
   .weak      DCMI_IRQHandler            
   .thumb_set DCMI_IRQHandler,Default_Handler
                                   
   .weak      HASH_RNG_IRQHandler                  
   .thumb_set HASH_RNG_IRQHandler,Default_Handler   

   .weak      FPU_IRQHandler                  
   .thumb_set FPU_IRQHandler,Default_Handler  
