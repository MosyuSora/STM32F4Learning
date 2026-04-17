/**
 * @file    system_stm32f4xx.c
 * @brief   系统初始化（最小版本）
 *
 * 启动文件 startup_stm32f407xx.s 的 Reset_Handler 在跳入 main() 之前
 * 会先调用这里的 SystemInit()。
 *
 * 完整版（HAL 工程里的版本）还负责外部 SRAM/SDRAM 初始化、
 * 向量表重定位等。当前工程用不到，全部省略。
 */
#include "stm32f4xx.h"

/* 全局变量：当前系统时钟频率（供库函数查询用） */
uint32_t SystemCoreClock = 16000000U;  /* 默认 HSI 16MHz */

/**
 * @brief 上电/复位后最早执行的 C 函数
 *
 * 在 Reset_Handler 完成 .data 搬运和 .bss 清零后、
 * 进入 main() 之前被调用。
 *
 * 这里只做一件事：使能 FPU（Cortex-M4 有硬件 FPU，
 * 上电默认是关闭的，需要手动打开才能使用浮点指令）。
 * SCB->CPACR 寄存器在 0xE000ED88，CP10 和 CP11 字段控制 FPU 访问权限。
 */
void SystemInit(void)
{
    /* 使能 FPU：CP10 和 CP11 设置为 Full Access（0b11） */
    *((volatile uint32_t *)0xE000ED88U) |= (0xFUL << 20U);
}
