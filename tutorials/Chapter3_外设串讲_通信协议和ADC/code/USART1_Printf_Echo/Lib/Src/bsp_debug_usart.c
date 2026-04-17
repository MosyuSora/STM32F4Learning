/**
 * @file    bsp_debug_usart.c
 * @brief   USART1 HAL 驱动实现
 *
 * 使用 HAL_UART_Receive_IT() 单字节接收模式：
 *   - 每收到 1 字节，HAL_UART_RxCpltCallback() 被调用
 *   - 回调中：写入环形缓冲区，立即回显，重新 arm 下一字节接收
 *
 * printf 重定向：实现 __io_putchar()，syscalls.c 中的 _write() 会调用它。
 */
#include "bsp_debug_usart.h"

UART_HandleTypeDef huart1;

/* 环形缓冲区（ISR 回调写，主循环读） */
static volatile uint8_t  rx_buf[RBUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static uint8_t rx_byte;   /* HAL 单字节接收缓存 */

/* ------------------------------------------------------------------
 * USART1_Config
 *   HAL_UART_Init() 初始化 USART1，然后启动第一次中断接收
 * ------------------------------------------------------------------ */
void USART1_Config(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = USART1_BAUD;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    /* 启动首次单字节中断接收 */
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/* ------------------------------------------------------------------
 * HAL_UART_MspInit  —  由 HAL_UART_Init() 内部回调
 *   负责 GPIO 时钟、引脚复用、NVIC 配置
 * ------------------------------------------------------------------ */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9  → USART1_TX，PA10 → USART1_RX，AF7 上拉推挽 */
        GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* NVIC：优先级 (1, 1) */
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/* ------------------------------------------------------------------
 * HAL_UART_RxCpltCallback  —  每收到 1 字节触发
 * ------------------------------------------------------------------ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uint16_t next = (rx_head + 1U) & (RBUF_SIZE - 1U);
        if (next != rx_tail) {          /* 缓冲区未满 */
            rx_buf[rx_head] = rx_byte;
            rx_head = next;
        }
        /* 立即回显 */
        HAL_UART_Transmit(&huart1, &rx_byte, 1, HAL_MAX_DELAY);

        /* 重新 arm，准备接收下一字节 */
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

/* ------------------------------------------------------------------
 * 环形缓冲区接口（主循环使用）
 * ------------------------------------------------------------------ */
uint8_t USART1_DataAvailable(void)
{
    return (rx_head != rx_tail) ? 1U : 0U;
}

uint8_t USART1_ReadByte(void)
{
    uint8_t ch = rx_buf[rx_tail];
    rx_tail    = (rx_tail + 1U) & (RBUF_SIZE - 1U);
    return ch;
}

/* ------------------------------------------------------------------
 * printf 重定向
 *   syscalls.c 中的 _write() 对每个字节调用 __io_putchar()
 * ------------------------------------------------------------------ */
int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
    return ch;
}
