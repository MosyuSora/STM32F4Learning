/**
 * @file    bsp_i2c_gpio.h
 * @brief   软件模拟 I²C GPIO 位操作接口（HAL 版本）
 *
 * 引脚：PB8 (SCL), PB9 (SDA)，开漏输出，外部 4.7 kΩ 上拉至 3.3 V
 * 速率：约 100 kbps（HAL_Delay 最小 1 ms；如需更快可改用 DWT 计数）
 */
#ifndef BSP_I2C_GPIO_H
#define BSP_I2C_GPIO_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* 位操作宏 */
#define I2C_SCL_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define I2C_SCL_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define I2C_SDA_HIGH()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)
#define I2C_SDA_LOW()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)
#define I2C_SDA_READ()  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9)

void    i2c_GPIO_Config(void);
void    i2c_Start(void);
void    i2c_Stop(void);
void    i2c_SendByte(uint8_t byte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void    i2c_Ack(void);
void    i2c_NAck(void);

#endif /* BSP_I2C_GPIO_H */
