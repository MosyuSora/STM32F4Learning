/**
 * @file    bsp_i2c_gpio.c
 * @brief   软件模拟 I²C 实现（HAL GPIO 版本）
 *
 * 使用 HAL_GPIO_WritePin/ReadPin 代替直接寄存器操作。
 * 软件延时改用 nop 循环（HAL_Delay 分辨率仅 1 ms，不适合位操作）。
 */
#include "bsp_i2c_gpio.h"

/* 软件延时 —— 控制 SCL 频率 */
static void i2c_Delay(void)
{
    volatile uint8_t n = 10;
    while (n--);
}

/* ------------------------------------------------------------------
 * i2c_GPIO_Config
 *   PB8(SCL), PB9(SDA) → 开漏输出，不接内部上拉
 *   初始状态：总线空闲（两线高电平）
 * ------------------------------------------------------------------ */
void i2c_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;   /* 开漏输出 */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;           /* 外部 4.7 kΩ 上拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_SCL_HIGH();
    I2C_SDA_HIGH();
}

/* ------------------------------------------------------------------
 * START 条件：SCL = H 时 SDA 下降沿
 * ------------------------------------------------------------------ */
void i2c_Start(void)
{
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    i2c_Delay();
    I2C_SDA_LOW();
    i2c_Delay();
    I2C_SCL_LOW();
    i2c_Delay();
}

/* ------------------------------------------------------------------
 * STOP 条件：SCL = H 时 SDA 上升沿
 * ------------------------------------------------------------------ */
void i2c_Stop(void)
{
    I2C_SDA_LOW();
    I2C_SCL_HIGH();
    i2c_Delay();
    I2C_SDA_HIGH();
    i2c_Delay();
}

/* ------------------------------------------------------------------
 * i2c_SendByte — 发送 8 位，MSB 优先
 * ------------------------------------------------------------------ */
void i2c_SendByte(uint8_t byte)
{
    for (int8_t i = 7; i >= 0; i--) {
        if (byte & (1U << i)) { I2C_SDA_HIGH(); }
        else                  { I2C_SDA_LOW();  }
        i2c_Delay();
        I2C_SCL_HIGH();
        i2c_Delay();
        I2C_SCL_LOW();
        i2c_Delay();
    }
}

/* ------------------------------------------------------------------
 * i2c_ReadByte — 读 8 位，MSB 优先（调用前释放 SDA）
 * ------------------------------------------------------------------ */
uint8_t i2c_ReadByte(void)
{
    uint8_t value = 0;
    I2C_SDA_HIGH();
    for (int8_t i = 7; i >= 0; i--) {
        I2C_SCL_HIGH();
        i2c_Delay();
        if (I2C_SDA_READ()) { value |= (1U << i); }
        I2C_SCL_LOW();
        i2c_Delay();
    }
    return value;
}

/* ------------------------------------------------------------------
 * i2c_WaitAck — 等待从机 ACK（返回 0: ACK, 1: NACK/超时）
 * ------------------------------------------------------------------ */
uint8_t i2c_WaitAck(void)
{
    uint8_t timeout = 250;
    I2C_SDA_HIGH();
    i2c_Delay();
    I2C_SCL_HIGH();
    i2c_Delay();
    while (I2C_SDA_READ()) {
        if (--timeout == 0) {
            i2c_Stop();
            return 1;
        }
    }
    I2C_SCL_LOW();
    i2c_Delay();
    return 0;
}

/* ------------------------------------------------------------------
 * i2c_Ack — 主机发 ACK（读中间字节后调用）
 * ------------------------------------------------------------------ */
void i2c_Ack(void)
{
    I2C_SDA_LOW();
    i2c_Delay();
    I2C_SCL_HIGH();
    i2c_Delay();
    I2C_SCL_LOW();
    i2c_Delay();
    I2C_SDA_HIGH();
}

/* ------------------------------------------------------------------
 * i2c_NAck — 主机发 NACK（读最后字节后调用）
 * ------------------------------------------------------------------ */
void i2c_NAck(void)
{
    I2C_SDA_HIGH();
    i2c_Delay();
    I2C_SCL_HIGH();
    i2c_Delay();
    I2C_SCL_LOW();
    i2c_Delay();
}
