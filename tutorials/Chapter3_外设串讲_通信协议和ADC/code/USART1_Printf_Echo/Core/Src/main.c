/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : USART1_Printf_Echo — HAL 版本
  *
  * 预期效果：打开串口终端（115200-8N1），看到启动信息，
  *           敲任意字符均被立即原样回显。
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "bsp_debug_usart.h"

/* USER CODE BEGIN 0 */

static void SystemClock_Config(void);

/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();

  /* USER CODE BEGIN 2 */
  USART1_Config();

  printf("\r\n===== USART1 Printf Echo Demo (HAL) =====\r\n");
  printf("Baud: %u-8N1  (HSI 16 MHz)\r\n", USART1_BAUD);
  printf("Type anything -- it will be echoed back.\r\n\r\n");
  /* USER CODE END 2 */

  while (1)
  {
    /* USER CODE BEGIN WHILE */
    /*
     * HAL_UART_RxCpltCallback() 已在中断中立即回显每个字节。
     * 主循环从环形缓冲区读取，做额外处理（如 \r -> \r\n 展开）。
     */
    if (USART1_DataAvailable()) {
      uint8_t ch = USART1_ReadByte();
      if (ch == '\r') { printf("\r\n"); }
    }
    /* USER CODE END WHILE */
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  System Clock Configuration — HSI 16 MHz, no PLL
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}

/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
