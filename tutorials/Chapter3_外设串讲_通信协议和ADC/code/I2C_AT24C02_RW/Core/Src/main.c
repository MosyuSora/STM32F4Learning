/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : I2C_AT24C02_RW — HAL 版本（软件模拟 I²C）
  *
  * 预期串口输出：
  *   ===== AT24C02 I2C EEPROM Test (HAL) =====
  *   Writing: Hello AT24C02! 0123
  *   Read back: Hello AT24C02! 0123
  *   Verify: [PASS]
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "bsp_debug_usart.h"
#include "bsp_i2c_ee.h"
#include <string.h>

/* USER CODE BEGIN 0 */
static void SystemClock_Config(void);
/* USER CODE END 0 */

#define TEST_ADDR   0x00U
#define TEST_LEN    20U

static const uint8_t write_buf[TEST_LEN] = {
    'H','e','l','l','o',' ','A','T','2','4',
    'C','0','2','!',' ','0','1','2','3','\0'
};
static uint8_t read_buf[TEST_LEN];

int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();
  SystemClock_Config();

  /* USER CODE BEGIN 2 */
  USART1_Config();
  ee_Init();

  printf("\r\n===== AT24C02 I2C EEPROM Test (HAL) =====\r\n");

  /* 写入 */
  printf("Writing: %s\r\n", (const char *)write_buf);
  if (ee_WriteBuffer(TEST_ADDR, write_buf, TEST_LEN)) {
      printf("Write FAILED (no ACK)!\r\n");
      while (1);
  }

  /* 读回 */
  memset(read_buf, 0, TEST_LEN);
  if (ee_ReadBuffer(TEST_ADDR, read_buf, TEST_LEN)) {
      printf("Read FAILED (no ACK)!\r\n");
      while (1);
  }

  /* 验证 */
  int pass = (memcmp(write_buf, read_buf, TEST_LEN) == 0);
  printf("Read back: %s\r\n", (const char *)read_buf);
  printf("Verify: %s\r\n", pass ? "[PASS]" : "[FAIL]");
  /* USER CODE END 2 */

  while (1) {}
}

/* USER CODE BEGIN 4 */
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
