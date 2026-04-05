/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* ========================= 时序参数 ========================= */
#define AUTO_SLEEP_MINUTES   1       /* 无操作 N 分钟后自动进入 Stop 模式         */
#define DEBOUNCE_MS          50      /* 物理按键软件消抖窗口 (ms)                 */
#define LONG_PRESS_MS        2000    /* SW5 长按判定阈值 (ms)                     */
#define BREATH_PERIOD_MS     2000    /* 单色呼吸周期：渐亮 1s + 渐灭 1s          */
#define SOLID_PERIOD_MS      2000    /* 纯色模式每色停留时间 (ms)                 */
#define RESET_BLINK_MS       200     /* 复位模式红灯闪烁半周期 (ms)               */

/* ========================= PWM 参数 ========================= */
#define COLOR_COUNT          7       /* 颜色总数（红黄绿青蓝品红白）              */
#define PWM_RESOLUTION       1000    /* PWM 步数 = ARR + 1                        */

/* ========================= 电容按键参数 ===================== */
#define CAP_TOUCH_THRESHOLD  50      /* 充电计数阈值，>= 此值判定为按下（需校准） */
#define CAP_CHARGE_TIMEOUT   200     /* 充电计数超时保护                          */
#define CAP_TOUCH_DEBOUNCE   300     /* 电容按键消抖间隔 (ms)                     */

/* ========================= IWDG 参数 ======================= */
#define IWDG_PRESCALER_DIV   IWDG_PRESCALER_64   /* 预分频 64                    */
#define IWDG_RELOAD_VALUE    4095                 /* 重装载值 (12-bit max)        */
/* 超时 ≈ (64 × 4096) / 32000 ≈ 8.19 s                                          */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
