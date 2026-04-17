/**
  ******************************************************************************
  * @file    bsp_sram.h
  * @brief   外部 SRAM (IS62WV51216) 驱动 - 头文件
  ******************************************************************************
  */
#ifndef __BSP_SRAM_H
#define __BSP_SRAM_H

#include "stm32f4xx_hal.h"

/* ── SRAM 基本参数 ── */
#define SRAM_BASE_ADDR    ((uint32_t)0x6C000000)   /* FSMC Bank1 NE4 */
#define SRAM_SIZE         (1024 * 1024)             /* 1MB */

/* ── 接口函数 ── */
void     BSP_SRAM_Init(void);
uint32_t BSP_SRAM_Test(void);

/* ── 便利读写函数 ── */
void     SRAM_WriteHalfWord(uint32_t offset, uint16_t data);
uint16_t SRAM_ReadHalfWord(uint32_t offset);
void     SRAM_WriteBuffer(uint32_t offset, uint16_t *buf, uint32_t count);
void     SRAM_ReadBuffer(uint32_t offset, uint16_t *buf, uint32_t count);

#endif /* __BSP_SRAM_H */
