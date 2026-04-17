/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "stm32f4xx_it.h"

/* USER CODE BEGIN EV */
extern UART_HandleTypeDef huart1;
/* USER CODE END EV */

void NMI_Handler(void)        { while (1) {} }
void HardFault_Handler(void)  { while (1) {} }
void MemManage_Handler(void)  { while (1) {} }
void BusFault_Handler(void)   { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void)        {}
void DebugMon_Handler(void)   {}
void PendSV_Handler(void)     {}
void SysTick_Handler(void)    { HAL_IncTick(); }

/* USER CODE BEGIN 1 */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}
/* USER CODE END 1 */
