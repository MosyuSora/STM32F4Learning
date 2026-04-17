/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : SPI_W25Q128_RW — HAL 版本
  *
  * 预期串口输出：
  *   ===== W25Q128 SPI Flash Test (HAL) =====
  *   JEDEC ID: 0xEF4018  [OK]
  *   Erasing sector 0 ...
  *   Erase done.
  *   Writing 16 bytes ...
  *   Read-back verify: [PASS]
  *   Data: DE AD BE EF 12 34 56 78 AB CD EF 01 23 45 67 89
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include "bsp_debug_usart.h"
#include "bsp_spi_flash.h"
#include <string.h>

/* USER CODE BEGIN 0 */
static void SystemClock_Config(void);
/* USER CODE END 0 */

#define TEST_ADDR    0x000000UL
#define TEST_LEN     16U

static const uint8_t write_buf[TEST_LEN] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78,
    0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89
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
  SPI_FLASH_Init();

  printf("\r\n===== W25Q128 SPI Flash Test (HAL) =====\r\n");

  /* 1. 读 JEDEC ID */
  uint32_t id = SPI_FLASH_ReadID();
  printf("JEDEC ID: 0x%06lX  %s\r\n", (unsigned long)id,
         (id == W25Q128_JEDEC_ID) ? "[OK]" : "[MISMATCH! Check wiring]");

  /* 2. 扇区擦除 */
  printf("Erasing sector 0 ...\r\n");
  SPI_FLASH_SectorErase(TEST_ADDR);
  printf("Erase done.\r\n");

  /* 3. 页编程 */
  printf("Writing %u bytes ...\r\n", TEST_LEN);
  SPI_FLASH_PageWrite(TEST_ADDR, write_buf, TEST_LEN);

  /* 4. 回读比较 */
  SPI_FLASH_BufferRead(TEST_ADDR, read_buf, TEST_LEN);
  int pass = (memcmp(write_buf, read_buf, TEST_LEN) == 0);
  printf("Read-back verify: %s\r\n", pass ? "[PASS]" : "[FAIL]");

  printf("Data: ");
  for (uint32_t i = 0; i < TEST_LEN; i++) {
      printf("%02X ", read_buf[i]);
  }
  printf("\r\n");
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
