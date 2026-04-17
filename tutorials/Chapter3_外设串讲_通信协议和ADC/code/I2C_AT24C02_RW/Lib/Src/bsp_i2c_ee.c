/**
 * @file    bsp_i2c_ee.c
 * @brief   AT24C02 应用层驱动实现（HAL 版本）
 *
 * 依赖 bsp_i2c_gpio.c 提供的位操作原语。
 * 页写完成等待改用 HAL_Delay(10)（AT24C02 最长 5 ms）。
 */
#include "bsp_i2c_ee.h"

void ee_Init(void)
{
    i2c_GPIO_Config();
}

/* ------------------------------------------------------------------
 * ee_WriteByte — 写单字节
 * ------------------------------------------------------------------ */
uint8_t ee_WriteByte(uint8_t mem_addr, uint8_t data)
{
    i2c_Start();
    i2c_SendByte(EEPROM_DEV_ADDR);
    if (i2c_WaitAck()) return 1;
    i2c_SendByte(mem_addr);
    if (i2c_WaitAck()) return 1;
    i2c_SendByte(data);
    if (i2c_WaitAck()) return 1;
    i2c_Stop();
    HAL_Delay(10);   /* AT24C02 内部编程最长 5 ms，留 2× 余量 */
    return 0;
}

/* ------------------------------------------------------------------
 * ee_WriteBuffer — 写多字节，自动处理 8 字节页边界
 * ------------------------------------------------------------------ */
uint8_t ee_WriteBuffer(uint8_t mem_addr, const uint8_t *buf, uint16_t len)
{
    while (len > 0) {
        uint8_t page_remain = (uint8_t)(EEPROM_PAGE_SIZE - (mem_addr % EEPROM_PAGE_SIZE));
        uint8_t write_len   = ((uint16_t)len < (uint16_t)page_remain)
                                ? (uint8_t)len : page_remain;

        i2c_Start();
        i2c_SendByte(EEPROM_DEV_ADDR);
        if (i2c_WaitAck()) return 1;
        i2c_SendByte(mem_addr);
        if (i2c_WaitAck()) return 1;

        for (uint8_t i = 0; i < write_len; i++) {
            i2c_SendByte(*buf++);
            if (i2c_WaitAck()) return 1;
        }
        i2c_Stop();
        HAL_Delay(10);

        mem_addr = (uint8_t)(mem_addr + write_len);
        len      = (uint16_t)(len    - write_len);
    }
    return 0;
}

/* ------------------------------------------------------------------
 * ee_ReadByte — 随机读单字节（先写虚帧定位，再 Repeated START）
 * ------------------------------------------------------------------ */
uint8_t ee_ReadByte(uint8_t mem_addr, uint8_t *data)
{
    i2c_Start();
    i2c_SendByte(EEPROM_DEV_ADDR);
    if (i2c_WaitAck()) return 1;
    i2c_SendByte(mem_addr);
    if (i2c_WaitAck()) return 1;

    i2c_Start();                            /* Repeated START */
    i2c_SendByte(EEPROM_DEV_ADDR | 0x01U);
    if (i2c_WaitAck()) return 1;

    *data = i2c_ReadByte();
    i2c_NAck();
    i2c_Stop();
    return 0;
}

/* ------------------------------------------------------------------
 * ee_ReadBuffer — 顺序读多字节
 * ------------------------------------------------------------------ */
uint8_t ee_ReadBuffer(uint8_t mem_addr, uint8_t *buf, uint16_t len)
{
    i2c_Start();
    i2c_SendByte(EEPROM_DEV_ADDR);
    if (i2c_WaitAck()) return 1;
    i2c_SendByte(mem_addr);
    if (i2c_WaitAck()) return 1;

    i2c_Start();
    i2c_SendByte(EEPROM_DEV_ADDR | 0x01U);
    if (i2c_WaitAck()) return 1;

    for (uint16_t i = 0; i < len; i++) {
        buf[i] = i2c_ReadByte();
        if (i < len - 1U) { i2c_Ack();  }
        else               { i2c_NAck(); }
    }
    i2c_Stop();
    return 0;
}
