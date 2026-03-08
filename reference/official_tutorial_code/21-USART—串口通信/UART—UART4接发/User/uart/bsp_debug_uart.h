#ifndef __DEBUG_UART_H
#define	__DEBUG_UART_H

#include "stm32f4xx.h"
#include <stdio.h>


//引脚定义
/*******************************************************/
#define DEBUG_UART                             UART4
#define DEBUG_UART_CLK                         RCC_APB1Periph_UART4
#define DEBUG_UART_BAUDRATE                    115200  //串口波特率

#define DEBUG_UART_RX_GPIO_PORT                GPIOC
#define DEBUG_UART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOC
#define DEBUG_UART_RX_PIN                      GPIO_Pin_11
#define DEBUG_UART_RX_AF                       GPIO_AF_UART4
#define DEBUG_UART_RX_SOURCE                   GPIO_PinSource11

#define DEBUG_UART_TX_GPIO_PORT                GPIOC
#define DEBUG_UART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOC
#define DEBUG_UART_TX_PIN                      GPIO_Pin_10
#define DEBUG_UART_TX_AF                       GPIO_AF_UART4
#define DEBUG_UART_TX_SOURCE                   GPIO_PinSource10

#define DEBUG_UART_IRQHandler                  UART4_IRQHandler
#define DEBUG_UART_IRQ                 				 UART4_IRQn
/************************************************************/

void Debug_UART_Config(void);
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);

void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch);

#endif /* __USART1_H */
