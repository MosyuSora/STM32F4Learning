/**
 * @file    bsp_spi_flash.h
 * @brief   W25Q128 SPI Flash 驱动接口（HAL 版本）
 *
 * 引脚分配（野火 STM32F407 开发板）：
 *   PB3 → SPI1_SCK  (AF5)
 *   PB4 → SPI1_MISO (AF5)
 *   PB5 → SPI1_MOSI (AF5)
 *   PG6 → CS        (普通推挽输出)
 */
#ifndef BSP_SPI_FLASH_H
#define BSP_SPI_FLASH_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* W25Q128 JEDEC ID: Manufacturer=0xEF, MemType=0x40, Capacity=0x18 */
#define W25Q128_JEDEC_ID    0xEF4018UL

/* W25Q128 指令集 */
#define W25X_WRITE_ENABLE       0x06U
#define W25X_WRITE_DISABLE      0x04U
#define W25X_READ_STATUS_REG    0x05U
#define W25X_PAGE_PROGRAM       0x02U
#define W25X_SECTOR_ERASE       0x20U
#define W25X_JEDEC_ID           0x9FU
#define W25X_READ_DATA          0x03U

/* CS 片选宏 */
#define FLASH_CS_LOW()   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET)
#define FLASH_CS_HIGH()  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET)

void     SPI_FLASH_Init(void);
uint8_t  SPI_FLASH_SendByte(uint8_t byte);
uint32_t SPI_FLASH_ReadID(void);
void     SPI_FLASH_SectorErase(uint32_t addr);
void     SPI_FLASH_PageWrite(uint32_t addr, const uint8_t *buf, uint16_t len);
void     SPI_FLASH_BufferRead(uint32_t addr, uint8_t *buf, uint32_t len);

#endif /* BSP_SPI_FLASH_H */
