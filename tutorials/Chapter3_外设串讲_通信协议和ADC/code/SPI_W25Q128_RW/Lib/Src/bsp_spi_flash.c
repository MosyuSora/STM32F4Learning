/**
 * @file    bsp_spi_flash.c
 * @brief   SPI1 + W25Q128 HAL 驱动实现
 *
 * 使用 HAL_SPI_TransmitReceive() 实现字节交换原语。
 * CS 片选仍为纯 GPIO 手动控制（HAL_GPIO_WritePin）。
 */
#include "bsp_spi_flash.h"

SPI_HandleTypeDef hspi1;

/* ------------------------------------------------------------------
 * SPI_FLASH_Init
 *   初始化 SPI1 和 CS GPIO
 * ------------------------------------------------------------------ */
void SPI_FLASH_Init(void)
{
    /* 1. CS 引脚单独初始化（PG6，推挽输出） */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitStruct.Pin   = GPIO_PIN_6;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    FLASH_CS_HIGH();

    /* 2. SPI1 HAL 初始化（HAL_SPI_MspInit 在回调中配置 GPIO AF） */
    hspi1.Instance               = SPI1;
    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;   /* CPOL=0 */
    hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;    /* CPHA=0 → Mode 0 */
    hspi1.Init.NSS               = SPI_NSS_SOFT;       /* 软件片选 */
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; /* 16/4 = 4 MHz */
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

/* ------------------------------------------------------------------
 * HAL_SPI_MspInit  —  由 HAL_SPI_Init() 内部回调
 * ------------------------------------------------------------------ */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* PB3=SCK, PB4=MISO, PB5=MOSI，AF5 */
        GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

/* ------------------------------------------------------------------
 * SPI_FLASH_SendByte  —  核心原语：交换一个字节（发送 + 接收）
 * ------------------------------------------------------------------ */
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

/* ------------------------------------------------------------------
 * 内部辅助：等待 Flash 内部操作完成（WIP=0）
 * ------------------------------------------------------------------ */
static void WaitWriteEnd(void)
{
    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_READ_STATUS_REG);
    while (SPI_FLASH_SendByte(0xFFU) & 0x01U);  /* WIP bit */
    FLASH_CS_HIGH();
}

/* ------------------------------------------------------------------
 * SPI_FLASH_ReadID  —  读 JEDEC ID（W25Q128 → 0xEF4018）
 * ------------------------------------------------------------------ */
uint32_t SPI_FLASH_ReadID(void)
{
    uint32_t id;
    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_JEDEC_ID);
    id  = (uint32_t)SPI_FLASH_SendByte(0xFFU) << 16U;
    id |= (uint32_t)SPI_FLASH_SendByte(0xFFU) <<  8U;
    id |=           SPI_FLASH_SendByte(0xFFU);
    FLASH_CS_HIGH();
    return id;
}

/* ------------------------------------------------------------------
 * SPI_FLASH_SectorErase  —  4 KB 扇区擦除
 * ------------------------------------------------------------------ */
void SPI_FLASH_SectorErase(uint32_t addr)
{
    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_WRITE_ENABLE);
    FLASH_CS_HIGH();

    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_SECTOR_ERASE);
    SPI_FLASH_SendByte((uint8_t)((addr >> 16U) & 0xFFU));
    SPI_FLASH_SendByte((uint8_t)((addr >>  8U) & 0xFFU));
    SPI_FLASH_SendByte((uint8_t)( addr         & 0xFFU));
    FLASH_CS_HIGH();

    WaitWriteEnd();
}

/* ------------------------------------------------------------------
 * SPI_FLASH_PageWrite  —  页编程（不能跨页边界，≤256 字节）
 * ------------------------------------------------------------------ */
void SPI_FLASH_PageWrite(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_WRITE_ENABLE);
    FLASH_CS_HIGH();

    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_PAGE_PROGRAM);
    SPI_FLASH_SendByte((uint8_t)((addr >> 16U) & 0xFFU));
    SPI_FLASH_SendByte((uint8_t)((addr >>  8U) & 0xFFU));
    SPI_FLASH_SendByte((uint8_t)( addr         & 0xFFU));
    for (uint16_t i = 0; i < len; i++) {
        SPI_FLASH_SendByte(buf[i]);
    }
    FLASH_CS_HIGH();

    WaitWriteEnd();
}

/* ------------------------------------------------------------------
 * SPI_FLASH_BufferRead  —  顺序读任意长度
 * ------------------------------------------------------------------ */
void SPI_FLASH_BufferRead(uint32_t addr, uint8_t *buf, uint32_t len)
{
    FLASH_CS_LOW();
    SPI_FLASH_SendByte(W25X_READ_DATA);
    SPI_FLASH_SendByte((uint8_t)((addr >> 16U) & 0xFFU));
    SPI_FLASH_SendByte((uint8_t)((addr >>  8U) & 0xFFU));
    SPI_FLASH_SendByte((uint8_t)( addr         & 0xFFU));
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = SPI_FLASH_SendByte(0xFFU);
    }
    FLASH_CS_HIGH();
}
