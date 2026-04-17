/**
  ******************************************************************************
  * @file    main.c
  * @brief   DMA UART TX 示例 — 用 DMA 发送字符串
  *
  *  演示内容：
  *    1. HAL_UART_Transmit_DMA() 非阻塞发送
  *    2. HAL_UART_TxCpltCallback() 发送完成回调
  *    3. 与轮询 / 中断方式的对比（可取消注释切换）
  ******************************************************************************
  */
#include "main.h"
#include <string.h>

/* ── 外设 Handle ── */
UART_HandleTypeDef  huart1;
DMA_HandleTypeDef   hdma_usart1_tx;

/* ── 发送缓冲区 ── */
uint8_t tx_buf[] = "Hello DMA! STM32F407 says hi.\r\n";

/* ── 标志位 ── */
volatile uint8_t tx_complete = 1;  /* 1=空闲可发, 0=正在发送 */

/* ── 函数声明 ── */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);

/* ====================================================================== */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();            /* ⚠️ DMA 必须在 USART 之前初始化 */
    MX_USART1_UART_Init();

    while (1)
    {
        /* ── 方式三：DMA 发送（默认） ── */
        if (tx_complete)
        {
            tx_complete = 0;
            HAL_UART_Transmit_DMA(&huart1, tx_buf, sizeof(tx_buf) - 1);
        }

        /*
         * CPU 现在是自由的！可以在这里做任何事情：
         * - 跑 PID 控制算法
         * - 刷新 LCD
         * - 检测按键状态机
         * - ...
         */

        HAL_Delay(1000);

        /* ── 方式一：轮询发送（取消注释试试） ── */
        // HAL_UART_Transmit(&huart1, tx_buf, sizeof(tx_buf) - 1, HAL_MAX_DELAY);
        // HAL_Delay(1000);

        /* ── 方式二：中断发送（取消注释试试） ── */
        // HAL_UART_Transmit_IT(&huart1, tx_buf, sizeof(tx_buf) - 1);
        // HAL_Delay(1000);
    }
}

/* ── DMA 发送完成回调 ── */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        tx_complete = 1;
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    }
}

/* ====================================================================== */
/*  以下为 CubeMX 生成的初始化代码（精简版，完整版由 CubeMX 生成）          */
/* ====================================================================== */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* LED */
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin   = LED_GREEN_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);
}

static void MX_DMA_Init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA2 Stream7 — USART1 TX */
    hdma_usart1_tx.Instance                 = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel             = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode                = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;  /* 直接模式 */
    HAL_DMA_Init(&hdma_usart1_tx);

    /* 关联到 USART1 */
    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

    /* NVIC */
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}

static void MX_USART1_UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    /* PA9 = TX, PA10 = RX */
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

    /* USART1 全局中断（中断方式发送时需要，DMA 方式也建议开启） */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* ── 中断服务函数 ── */
void DMA2_Stream7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/* ── 系统时钟配置：HSE → PLL → 168MHz ── */
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
