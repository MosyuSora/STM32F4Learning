/**
 * @file    rcc.h
 * @brief   RCC 时钟使能驱动（最小版本）
 *
 * 当前只封装了本工程用到的 AHB1 总线 GPIO 时钟使能函数。
 * 实际项目中 RCC 还负责时钟源切换、PLL 配置、分频等，
 * 那些功能会在后续章节的 HAL 版本中深入讨论。
 */
#ifndef __RCC_H
#define __RCC_H

#include "stm32f4xx.h"

/* AHB1 总线 GPIO 时钟使能 */
void RCC_GPIO_ClkEnable(GPIO_TypeDef *GPIOx);

#endif /* __RCC_H */
