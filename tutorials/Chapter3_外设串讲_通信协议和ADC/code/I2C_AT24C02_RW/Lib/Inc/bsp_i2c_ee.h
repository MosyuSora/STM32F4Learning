/**
 * @file    bsp_i2c_ee.h
 * @brief   AT24C02 EEPROM 应用层接口（HAL 版本）
 */
#ifndef BSP_I2C_EE_H
#define BSP_I2C_EE_H

#include "bsp_i2c_gpio.h"
#include <stdint.h>

/* AT24C02: 7-bit 地址 0x50（A2A1A0=000），写=0xA0，读=0xA1 */
#define EEPROM_DEV_ADDR   0xA0U
#define EEPROM_PAGE_SIZE  8U

void    ee_Init(void);
uint8_t ee_WriteByte(uint8_t mem_addr, uint8_t data);
uint8_t ee_WriteBuffer(uint8_t mem_addr, const uint8_t *buf, uint16_t len);
uint8_t ee_ReadByte(uint8_t mem_addr, uint8_t *data);
uint8_t ee_ReadBuffer(uint8_t mem_addr, uint8_t *buf, uint16_t len);

#endif /* BSP_I2C_EE_H */
