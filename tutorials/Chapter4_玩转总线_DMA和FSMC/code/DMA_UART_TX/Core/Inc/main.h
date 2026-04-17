/**
  ******************************************************************************
  * @file    main.h
  * @brief   DMA UART TX 示例 - 头文件
  ******************************************************************************
  */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void Error_Handler(void);

/* ── 用户定义 ── */
#define LED_GREEN_Pin       GPIO_PIN_10
#define LED_GREEN_GPIO_Port GPIOF

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
