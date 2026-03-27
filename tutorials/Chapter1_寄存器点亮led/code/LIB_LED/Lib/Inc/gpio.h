/**
 * @file    gpio.h
 * @brief   GPIO 驱动（最小版本）
 *
 * 对比 HAL 的 stm32f4xx_hal_gpio.h，这里只保留了点灯需要的部分：
 *   - GPIO_InitTypeDef：引脚配置结构体
 *   - GPIO_PinState：引脚电平枚举
 *   - GPIO_Init / GPIO_WritePin / GPIO_TogglePin / GPIO_ReadPin
 *
 * 用结构体抽象的好处：
 *   1. 调用方不需要记住各字段在寄存器里的 bit 位置
 *   2. 同一套 API 可以复用到所有 GPIO 端口（GPIOA..GPIOI）
 *   3. 接口和实现分离，以后换芯片只需改 .c，头文件不变
 */
#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f4xx.h"

/* ── 引脚编号 ──────────────────────────────────────────────────── */
#define GPIO_PIN_0   (1U << 0)
#define GPIO_PIN_1   (1U << 1)
#define GPIO_PIN_2   (1U << 2)
#define GPIO_PIN_3   (1U << 3)
#define GPIO_PIN_4   (1U << 4)
#define GPIO_PIN_5   (1U << 5)
#define GPIO_PIN_6   (1U << 6)
#define GPIO_PIN_7   (1U << 7)
#define GPIO_PIN_8   (1U << 8)
#define GPIO_PIN_9   (1U << 9)
#define GPIO_PIN_10  (1U << 10)
#define GPIO_PIN_11  (1U << 11)
#define GPIO_PIN_12  (1U << 12)
#define GPIO_PIN_13  (1U << 13)
#define GPIO_PIN_14  (1U << 14)
#define GPIO_PIN_15  (1U << 15)
#define GPIO_PIN_ALL (0xFFFFU)

/* ── 引脚模式（对应 MODER 寄存器 2-bit 字段） ──────────────────── */
#define GPIO_MODE_INPUT     0x00U   /*!< 输入模式        */
#define GPIO_MODE_OUTPUT_PP 0x01U   /*!< 推挽输出        */
#define GPIO_MODE_OUTPUT_OD 0x11U   /*!< 开漏输出（高4位=1标识OD）*/
#define GPIO_MODE_AF_PP     0x02U   /*!< 复用推挽        */
#define GPIO_MODE_AF_OD     0x12U   /*!< 复用开漏        */
#define GPIO_MODE_ANALOG    0x03U   /*!< 模拟模式        */

/* ── 上下拉（对应 PUPDR 寄存器 2-bit 字段） ────────────────────── */
#define GPIO_NOPULL   0x00U /*!< 无上下拉 */
#define GPIO_PULLUP   0x01U /*!< 上拉     */
#define GPIO_PULLDOWN 0x02U /*!< 下拉     */

/* ── 输出速度（对应 OSPEEDR 寄存器 2-bit 字段） ─────────────────── */
#define GPIO_SPEED_FREQ_LOW       0x00U /*!< 低速  (~2MHz)  */
#define GPIO_SPEED_FREQ_MEDIUM    0x01U /*!< 中速  (~25MHz) */
#define GPIO_SPEED_FREQ_HIGH      0x02U /*!< 高速  (~50MHz) */
#define GPIO_SPEED_FREQ_VERY_HIGH 0x03U /*!< 极高速(~100MHz)*/

/* ── 引脚电平枚举 ───────────────────────────────────────────────── */
typedef enum {
    GPIO_PIN_RESET = 0, /*!< 低电平（RGB LED 共阳极：灯亮） */
    GPIO_PIN_SET   = 1  /*!< 高电平（RGB LED 共阳极：灯灭） */
} GPIO_PinState;

/* ── 初始化配置结构体 ───────────────────────────────────────────── */
/**
 * @brief 引脚初始化参数
 *
 * 调用方填写这个结构体，再传给 GPIO_Init()。
 * 这样就避免了把 MODER/OTYPER/OSPEEDR/PUPDR 四个寄存器分开设置，
 * 让初始化代码读起来像"配置表"而不是"寄存器操作流水账"。
 */
typedef struct {
    uint32_t Pin;   /*!< 引脚编号，可用 | 同时指定多个，如 GPIO_PIN_6|GPIO_PIN_7 */
    uint32_t Mode;  /*!< 引脚模式，见 GPIO_MODE_xxx 定义                         */
    uint32_t Pull;  /*!< 上下拉，见 GPIO_NOPULL / GPIO_PULLUP / GPIO_PULLDOWN     */
    uint32_t Speed; /*!< 输出速度，见 GPIO_SPEED_FREQ_xxx 定义                    */
} GPIO_InitTypeDef;

/* ── 函数声明 ───────────────────────────────────────────────────── */
void          GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void          GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void          GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
GPIO_PinState GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif /* __GPIO_H */
