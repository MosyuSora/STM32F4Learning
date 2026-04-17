/**
  ******************************************************************************
  * @file    main.c
  * @brief   FSMC SRAM 读写测试 — 外部 IS62WV51216 (1MB)
  *
  *  演示内容：
  *    1. BSP_SRAM_Init() 初始化 FSMC + GPIO
  *    2. BSP_SRAM_Test() 全地址校验
  *    3. 指针直接读写外部 SRAM
  *    4. 通过 USART1 打印测试结果
  ******************************************************************************
  */
#include "main.h"
#include "bsp_sram.h"
#include <stdio.h>
#include <string.h>

/* ── 外设 Handle ── */
UART_HandleTypeDef huart1;

/* ── 函数声明 ── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* ── 重定向 printf 到 USART1 ── */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* ====================================================================== */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    printf("\r\n====== FSMC SRAM Test ======\r\n");
    printf("SRAM Base: 0x%08lX, Size: %lu KB\r\n", SRAM_BASE_ADDR, SRAM_SIZE / 1024);

    /* ── 初始化 FSMC ── */
    BSP_SRAM_Init();
    printf("FSMC init done.\r\n");

    /* ── 全地址校验 ── */
    printf("Running full address test (1MB)...\r\n");
    uint32_t result = BSP_SRAM_Test();
    if (result == 0)
    {
        printf("SRAM test PASSED! All 512K half-words verified.\r\n");
    }
    else
    {
        printf("SRAM test FAILED at offset 0x%08lX\r\n", result);
    }

    /* ── 简单读写演示 ── */
    printf("\r\n--- Read/Write Demo ---\r\n");
    SRAM_WriteHalfWord(0x0000, 0xBEEF);
    SRAM_WriteHalfWord(0x0002, 0xCAFE);
    printf("Write: [0x0000]=0xBEEF, [0x0002]=0xCAFE\r\n");
    printf("Read:  [0x0000]=0x%04X, [0x0002]=0x%04X\r\n",
           SRAM_ReadHalfWord(0x0000), SRAM_ReadHalfWord(0x0002));

    /* ── 大块写入演示 ── */
    uint16_t buf_w[8] = {0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888};
    uint16_t buf_r[8] = {0};
    SRAM_WriteBuffer(0x1000, buf_w, 8);
    SRAM_ReadBuffer(0x1000, buf_r, 8);
    printf("\r\nBuffer write/read at 0x1000:\r\n  ");
    for (int i = 0; i < 8; i++)
        printf("0x%04X ", buf_r[i]);
    printf("\r\n");

    printf("\r\n====== Test Complete ======\r\n");

    while (1)
    {
        HAL_Delay(1000);
    }
}

/* ====================================================================== */
/*  初始化代码                                                            */
/* ====================================================================== */

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void MX_USART1_UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 25;
    RCC_OscInitStruct.PLL.PLLN       = 336;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
