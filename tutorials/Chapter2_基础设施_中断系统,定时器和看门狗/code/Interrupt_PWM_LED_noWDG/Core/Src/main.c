/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/*
 * ========================= 颜色占空比表 =========================
 * 每行 = { R%, G%, B% }，百分比指该通道在最亮时的占空比。
 * 例如：黄色 = 红50% + 绿50%，白色 = 三通道各33%。
 *
 * LED 低电平点亮（低有效），PWM Mode1 + 极性HIGH 意味着：
 *   CNT < CCR → 引脚 HIGH → LED 灭
 *   CNT ≥ CCR → 引脚 LOW  → LED 亮
 * 所以 CCR = 0 → LED 全亮，CCR = 1000 → LED 全灭。
 * 亮度值 b (0~1000) 对应 CCR = 1000 - b。
 */
typedef struct {
    uint16_t r;    /* 红色通道亮度百分比 0-100 */
    uint16_t g;    /* 绿色通道亮度百分比 0-100 */
    uint16_t b;    /* 蓝色通道亮度百分比 0-100 */
} Color_t;

static const Color_t COLOR_TABLE[COLOR_COUNT] = {
    {100,   0,   0},   /* 红色 */
    { 50,  50,   0},   /* 黄色 */
    {  0, 100,   0},   /* 绿色 */
    {  0,  50,  50},   /* 青色 */
    {  0,   0, 100},   /* 蓝色 */
    { 50,   0,  50},   /* 品红 */
    { 33,  33,  33},   /* 白色 */
};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;
TIM_HandleTypeDef htim13;

/* USER CODE BEGIN PV */
static volatile uint16_t dbg_ccr_r=0;

static volatile uint16_t dbg_ccr_g=0;
	
static volatile uint16_t dbg_ccr_b=0;
/* ===== 运行模式 ===== */
typedef enum {
    MODE_BREATHING = 0,   /* 模式1：七色呼吸灯 */
    MODE_SOLID     = 1,   /* 模式2：七色纯色循环 */
    MODE_RESET     = 2,   /* 复位模式：红灯快闪 */
} RunMode_t;

static volatile RunMode_t g_currentMode  = MODE_BREATHING;
static volatile bool      g_inResetMode  = false;   /* 是否处于复位模式 */

/* ===== TIM6 空闲计时（每秒 +1，达到阈值进入 Stop） ===== */
static volatile uint32_t  g_idleSeconds  = 0;
static volatile bool      g_enterStop    = false;   /* Stop 请求标志 */

/* ===== SW5 物理按键状态 ===== */
static volatile uint32_t  g_sw5PressTime   = 0;     /* 按下时刻 (tick) */
static volatile uint32_t  g_sw5ReleaseTime = 0;     /* 松开时刻 (tick) */
static volatile bool      g_sw5Pressed     = false; /* 当前是否处于按下 */

/* ===== 颜色 / 呼吸状态 ===== */
static uint8_t   g_colorIndex     = 0;               /* 当前颜色索引 0~6 */
static uint32_t  g_lastColorTick  = 0;               /* 上次颜色切换 tick */

/* ===== 电容按键 ===== */
static uint32_t  g_lastCapTouchTick = 0;             /* 上次触发 tick（消抖）*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM11_Init(void);
static void MX_TIM13_Init(void);
/* USER CODE BEGIN PFP */

/* LED 控制 */
static void LED_SetRGB(uint16_t r, uint16_t g, uint16_t b);
static void LED_Off(void);
static void LED_UpdateBreathing(void);
static void LED_UpdateSolid(void);
static void LED_UpdateReset(void);

/* 电容按键 */
static bool CapTouch_Detect(void);

/* 电源管理 */
static void EnterStopMode(void);

/* 工具 */
static void ResetIdleCounter(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM6_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_TIM13_Init();
  /* USER CODE BEGIN 2 */

  /* ---------- 启动三路 PWM 输出 ----------
   * 初始 CCR = 0（CubeMX 默认 Pulse = 0），对应引脚持续 LOW → LED 全亮。
   * 我们先调用 LED_Off() 把 CCR 设为 1000，让 LED 初始为灭。 */
  HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);   /* PF6 红 */
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);   /* PF7 绿 */
  HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);   /* PF8 蓝 */
  LED_Off();

  /* ---------- 启动 TIM6 中断（1s 周期）---------- */
  HAL_TIM_Base_Start_IT(&htim6);

  /* ---------- 使能 EXTI3 事件线 (EMR) ----------
   * CubeMX 只配置了 IMR（中断线），这里手动补上 EMR，
   * 让 SW5 在 Stop 模式下也可通过事件唤醒 CPU (WFE)。 */
  EXTI->EMR |= EXTI_EMR_MR3;

  /* 记录起始 tick，用于呼吸 / 纯色的时间基准 */
  g_lastColorTick = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* ========== 1. 检查 Stop 模式请求 ========== */
    if (g_enterStop)
    {
      EnterStopMode();
      continue;   /* 唤醒后从循环头重新开始 */
    }

    /* ========== 2. SW5 长按检测 ========== 
     * ISR 只记录时间戳，这里在主循环里判断按压持续时间。
     * 松开后 (g_sw5Pressed == false) 且 ReleaseTime > PressTime → 有效释放。 */
    if (!g_sw5Pressed && g_sw5ReleaseTime > g_sw5PressTime)
    {
      uint32_t duration = g_sw5ReleaseTime - g_sw5PressTime;
      g_sw5ReleaseTime = 0;   /* 消费掉这次事件，防止重复处理 */

      if (duration >= LONG_PRESS_MS)
      {
        /* 长按 >= 5s → 切换复位模式 / 普通模式 */
        g_inResetMode = !g_inResetMode;
        if (g_inResetMode)
        {
          g_currentMode = MODE_RESET;
        }
        else
        {
          g_currentMode = MODE_BREATHING;
          g_colorIndex  = 0;
          g_lastColorTick = HAL_GetTick();
        }
      }
    }

    /* ========== 3. 电容按键检测 → 切换呼吸 / 纯色 ========== 
     * 只在普通模式下响应（复位模式忽略触摸） */
    if (!g_inResetMode)
    {
      if (CapTouch_Detect())
      {
        uint32_t now = HAL_GetTick();
        if (now - g_lastCapTouchTick >= CAP_TOUCH_DEBOUNCE)
        {
          g_lastCapTouchTick = now;
          ResetIdleCounter();

          /* 呼吸 ↔ 纯色 互相切换 */
          if (g_currentMode == MODE_BREATHING)
            g_currentMode = MODE_SOLID;
          else
            g_currentMode = MODE_BREATHING;

          g_colorIndex    = 0;
          g_lastColorTick = HAL_GetTick();
        }
      }
    }

    /* ========== 4. 根据当前模式刷新 LED ========== */
    switch (g_currentMode)
    {
      case MODE_BREATHING:
        LED_UpdateBreathing();
        break;
      case MODE_SOLID:
        LED_UpdateSolid();
        break;
      case MODE_RESET:
        LED_UpdateReset();
        break;
    }
    dbg_ccr_r = TIM10->CCR1;//给Debugger用的 观察LED的PWM
    dbg_ccr_g = TIM11->CCR1;
    dbg_ccr_b = TIM13->CCR1;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 167;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 999;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */
  HAL_TIM_MspPostInit(&htim10);

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 167;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 999;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim11, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */
  HAL_TIM_MspPostInit(&htim11);

}

/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(void)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 83;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = 999;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim13, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */

  /* USER CODE END TIM13_Init 2 */
  HAL_TIM_MspPostInit(&htim13);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PG3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* =====================================================================
 *  LED 控制函数
 * =====================================================================
 * 硬件约束（低有效 + PWM Mode1 + 极性HIGH）：
 *   CCR = 0    → 引脚始终 LOW  → LED 全亮 (100%)
 *   CCR = 1000 → 引脚始终 HIGH → LED 全灭 (0%)
 *   亮度 b (0~1000) 对应 CCR = PWM_RESOLUTION - b
 * ===================================================================== */

/**
 * @brief  设置 RGB 三通道亮度
 * @param  r  红色亮度 0~1000 (0=灭, 1000=最亮)
 * @param  g  绿色亮度 0~1000
 * @param  b  蓝色亮度 0~1000
 */
static void LED_SetRGB(uint16_t r, uint16_t g, uint16_t b)
{
  __HAL_TIM_SET_COMPARE(&htim10, TIM_CHANNEL_1, PWM_RESOLUTION - r); /* PF6 红 */
  __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, PWM_RESOLUTION - g); /* PF7 绿 */
  __HAL_TIM_SET_COMPARE(&htim13, TIM_CHANNEL_1, PWM_RESOLUTION - b); /* PF8 蓝 */
}

/** @brief 关闭所有 LED */
static void LED_Off(void)
{
  LED_SetRGB(0, 0, 0);
}

/**
 * @brief  模式1 -- 七色呼吸灯
 *
 * 每个颜色的呼吸周期 = BREATH_PERIOD_MS (2000ms)：
 *   前半段 0~999ms  ：亮度从 0 线性递增到 1000（渐亮）
 *   后半段 1000~1999ms：亮度从 1000 线性递减到 0（渐灭）
 *
 * 用 HAL_GetTick() 计算经过时间，得到亮度系数 (0~1000)，
 * 再乘以颜色表中该通道的百分比，得到最终 PWM 值。
 */
static void LED_UpdateBreathing(void)
{
  uint32_t now     = HAL_GetTick();
  uint32_t elapsed = now - g_lastColorTick;

  /* 一个颜色的呼吸周期结束 → 切换下一色 */
  if (elapsed >= BREATH_PERIOD_MS)
  {
    g_colorIndex    = (g_colorIndex + 1) % COLOR_COUNT;
    g_lastColorTick = now;
    elapsed = 0;
  }

  /* 计算亮度系数 brightness ∈ [0, 1000]
   *   前半周期：线性 0 → 1000
   *   后半周期：线性 1000 → 0 */
  uint32_t half = BREATH_PERIOD_MS / 2;
  uint16_t brightness;
  if (elapsed < half)
    brightness = (uint16_t)(elapsed * PWM_RESOLUTION / half);
  else
    brightness = (uint16_t)((BREATH_PERIOD_MS - elapsed) * PWM_RESOLUTION / half);

  /* 颜色百分比 × 亮度系数 → 通道值 0~1000 */
  const Color_t *c = &COLOR_TABLE[g_colorIndex];
  uint16_t r = (uint16_t)((uint32_t)c->r * brightness / 100);
  uint16_t g = (uint16_t)((uint32_t)c->g * brightness / 100);
  uint16_t b = (uint16_t)((uint32_t)c->b * brightness / 100);

  LED_SetRGB(r, g, b);
}

/**
 * @brief  模式2 -- 七色纯色循环
 *
 * 七种颜色以最大亮度显示，每 SOLID_PERIOD_MS 切换下一色。
 */
static void LED_UpdateSolid(void)
{
  uint32_t now = HAL_GetTick();
  if (now - g_lastColorTick >= SOLID_PERIOD_MS)
  {
    g_colorIndex    = (g_colorIndex + 1) % COLOR_COUNT;
    g_lastColorTick = now;
  }

  const Color_t *c = &COLOR_TABLE[g_colorIndex];
  uint16_t r = (uint16_t)((uint32_t)c->r * PWM_RESOLUTION / 100);
  uint16_t g = (uint16_t)((uint32_t)c->g * PWM_RESOLUTION / 100);
  uint16_t b = (uint16_t)((uint32_t)c->b * PWM_RESOLUTION / 100);

  LED_SetRGB(r, g, b);
}

/**
 * @brief  复位模式 -- 红灯以 RESET_BLINK_MS 为半周期快闪
 */
static void LED_UpdateReset(void)
{
  uint32_t phase = (HAL_GetTick() / RESET_BLINK_MS) % 2;
  if (phase == 0)
    LED_SetRGB(PWM_RESOLUTION, 0, 0);  /* 红灯全亮 */
  else
    LED_Off();                          /* 全灭     */
}

/* =====================================================================
 *  电容按键检测
 * =====================================================================
 * PA5 连接电容触摸焊盘，等效为一个对地 RC 网络。
 * 检测流程：
 *   1. PA5 配置为推挽输出，拉低 → 给焊盘电容放电
 *   2. PA5 切换为带内部上拉的输入 → 电容通过上拉电阻开始充电
 *   3. 循环读取引脚电平，计数从 LOW 变 HIGH 所需的循环次数
 *   4. 手指按下 → 并联人体电容 → RC 时间常数增大 → 计数变大
 *   5. 计数 >= CAP_TOUCH_THRESHOLD → 判定为按下
 *
 * 注意：CubeMX 把 PA5 配成了 TIM2_CH1 AF，这里每次检测时
 *       重新配置为 GPIO，不影响功能。阈值需根据实际硬件校准。
 * ===================================================================== */
static bool CapTouch_Detect(void)
{
  GPIO_InitTypeDef gpio = {0};
  uint16_t count = 0;

  /* ---- 第一步：推挽输出 LOW → 放电 ---- */
  gpio.Pin   = GPIO_PIN_5;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /* 等待放电完成（约几百 ns，循环耗时已足够） */
  for (volatile uint32_t i = 0; i < 200; i++) { __NOP(); }

  /* ---- 第二步：切换为上拉输入 → 开始充电 ---- */
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &gpio);

  /* ---- 第三步：计数直到引脚变 HIGH ---- */
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)
  {
    count++;
    if (count >= CAP_CHARGE_TIMEOUT)
      break;                          /* 超时保护 */
  }

  return (count >= CAP_TOUCH_THRESHOLD);
}

/* =====================================================================
 *  电源管理 —— 进入 / 退出 Stop 模式
 * ===================================================================== */

/**
 * @brief  进入 Stop 模式
 *
 * 1. 关闭 LED + 停止 PWM / TIM6
 * 2. HAL_SuspendTick() —— 防止 HSI 低速下 tick 计数失真
 * 3. HAL_PWR_EnterSTOPMode(WFE) —— CPU 暂停于此
 *          ↓  SW5 按下 → EXTI3 事件唤醒
 * 4. SystemClock_Config() —— 恢复 PLL → 168MHz
 * 5. HAL_ResumeTick() + 重新启动外设
 */
static void EnterStopMode(void)
{
  /* 关灯 + 停外设 */
  LED_Off();
  HAL_TIM_PWM_Stop(&htim10, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&htim11, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&htim13, TIM_CHANNEL_1);
  HAL_TIM_Base_Stop_IT(&htim6);

  /* 暂停 SysTick，防止在 HSI 16MHz 下产生错误 tick */
  HAL_SuspendTick();

  /* ====== 进入 Stop 模式 (低功耗稳压器 + WFE) ======
   * CPU 在此行暂停，直到 EXTI3 事件把它唤醒 */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);

  /* ====== 唤醒后从这里继续 ======
   * Stop 期间 HSE/PLL 被硬件关闭，芯片回退到 HSI 16MHz，
   * 必须先恢复时钟树，否则 USART/TIM/SysTick 频率全乱。 */
  SystemClock_Config();
  HAL_ResumeTick();

  /* 重新启动 PWM 和 TIM6 */
  HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim6);

  /* 重置状态 */
  g_idleSeconds   = 0;
  g_enterStop     = false;
  g_lastColorTick = HAL_GetTick();
}

/** @brief 重置空闲计时器（任何用户交互时调用） */
static void ResetIdleCounter(void)
{
  g_idleSeconds = 0;
}

/* =====================================================================
 *  HAL 回调函数
 * =====================================================================
 * HAL 库对 TIM_PeriodElapsedCallback 和 GPIO_EXTI_Callback 使用 __weak
 * 默认空实现。我们在这里重写它们，链接器会自动选择非 weak 版本。
 * 中断处理流程：
 *   EXTI3_IRQHandler → HAL_GPIO_EXTI_IRQHandler → HAL_GPIO_EXTI_Callback
 *   TIM6_DAC_IRQHandler → HAL_TIM_IRQHandler → HAL_TIM_PeriodElapsedCallback
 * ===================================================================== */

/**
 * @brief  TIM 更新事件回调（TIM6 每秒触发一次）
 *
 * 累加空闲秒数。达到 AUTO_SLEEP_MINUTES 后置标志，
 * 由主循环负责执行 Stop 进入流程（ISR 中不做复杂操作）。
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
    g_idleSeconds++;
    if (g_idleSeconds >= (uint32_t)AUTO_SLEEP_MINUTES * 60)
    {
      g_enterStop = true;
    }
  }
}

/**
 * @brief  EXTI 外部中断回调（SW5 物理按键 PG3）
 *
 * CubeMX 配置为双边沿触发：按下（下降沿）和松开（上升沿）都会进入。
 * ISR 只记录时间戳和当前按键状态（快进快出），长按判定留给主循环。
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_3)
  {
    uint32_t now = HAL_GetTick();

    if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET)
    {
      /* 下降沿 → 按键按下 */
      if (now - g_sw5PressTime >= DEBOUNCE_MS)    /* 消抖 */
      {
        g_sw5PressTime = now;
        g_sw5Pressed   = true;
      }
    }
    else
    {
      /* 上升沿 → 按键松开 */
      if (now - g_sw5ReleaseTime >= DEBOUNCE_MS)  /* 消抖 */
      {
        g_sw5ReleaseTime = now;
        g_sw5Pressed     = false;
      }
    }

    /* 有按键操作 → 重置空闲计时 */
    ResetIdleCounter();
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
