/**
 * @file    bsp_debug_usart.h
 * @brief   USART1 HAL 驱动接口
 *
 * 引脚：PA9 (TX, AF7)，PA10 (RX, AF7)
 * 时钟：APB2，HSI 16 MHz，波特率 115200
 *
 * printf 重定向：实现 __io_putchar()，由 syscalls.c 中的 _write() 调用。
 * 接收：中断模式，每次接收一字节，HAL_UART_RxCpltCallback() 写环形缓冲区。
 */
#ifndef BSP_DEBUG_USART_H
#define BSP_DEBUG_USART_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdio.h>

#define USART1_BAUD   115200U
#define RBUF_SIZE     64U    /* 必须是 2 的幂 */

void    USART1_Config(void);
uint8_t USART1_DataAvailable(void);
uint8_t USART1_ReadByte(void);

#endif /* BSP_DEBUG_USART_H */

