/**
 * @file    gpio.c
 * @brief   GPIO 驱动实现（最小版本）
 *
 * 这里所做的事情和汇编版 led.s 的 GPIO 初始化段完全等价，
 * 只是从"手算 bit 位，直接 str 寄存器"变成了
 * "通过结构体字段名访问，让编译器算 offset"。
 *
 * 对比标准库（STM32F4xx_StdPeriph_Driver）的 stm32f4xx_gpio.c，
 * 这里裁掉了：AF 复用配置、EXTI 配置、Lock 功能。
 * 保留了：推挽/开漏输出初始化、WritePin、TogglePin、ReadPin。
 */
#include "gpio.h"

/**
 * @brief 初始化一个或多个 GPIO 引脚
 *
 * 遍历 GPIO_Init->Pin 里的每一个置位 bit，
 * 对每一个引脚依次配置 MODER→OTYPER→OSPEEDR→PUPDR。
 *
 * 为什么用"读-改-写"而不是直接覆盖整个寄存器？
 * 因为 MODER/OTYPER/OSPEEDR/PUPDR 每个寄存器管 16 个引脚，
 * 一次初始化通常只改其中几个，直接覆盖会破坏其他引脚的配置。
 */
void GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    uint32_t pos = 0U;
    uint32_t temp;

    /* 遍历 16 个引脚 bit */
    while ((GPIO_Init->Pin >> pos) != 0U) {
        /* 当前 bit 是否在 GPIO_Init->Pin 里被选中 */
        if ((GPIO_Init->Pin & (1UL << pos)) == 0U) {
            pos++;
            continue;
        }

        /* ── 1. MODER：每引脚 2 bit，pos*2 定位 ─────────────────── */
        temp  = GPIOx->MODER;
        temp &= ~(0x3UL << (pos * 2U));               /* 清除原值 */
        temp |=  ((GPIO_Init->Mode & 0x03U) << (pos * 2U)); /* 写新值 */
        GPIOx->MODER = temp;

        /* ── 2. OTYPER：每引脚 1 bit（只对输出/复用模式有效）────── */
        if ((GPIO_Init->Mode & 0x03U) != GPIO_MODE_INPUT &&
            (GPIO_Init->Mode & 0x03U) != GPIO_MODE_ANALOG) {
            temp  = GPIOx->OTYPER;
            temp &= ~(1UL << pos);
            temp |=  (((GPIO_Init->Mode >> 4U) & 0x1U) << pos);
            GPIOx->OTYPER = temp;
        }

        /* ── 3. OSPEEDR：每引脚 2 bit ───────────────────────────── */
        temp  = GPIOx->OSPEEDR;
        temp &= ~(0x3UL << (pos * 2U));
        temp |=  (GPIO_Init->Speed << (pos * 2U));
        GPIOx->OSPEEDR = temp;

        /* ── 4. PUPDR：每引脚 2 bit ─────────────────────────────── */
        temp  = GPIOx->PUPDR;
        temp &= ~(0x3UL << (pos * 2U));
        temp |=  (GPIO_Init->Pull << (pos * 2U));
        GPIOx->PUPDR = temp;

        pos++;
    }
}

/**
 * @brief 设置引脚电平
 *
 * 使用 BSRR 寄存器而不是 ODR，原因：
 *   - 置位：写低16位（BS[x]=1）→ ODR[x]=1，不影响其他位
 *   - 复位：写高16位（BR[x]=1）→ ODR[x]=0，不影响其他位
 *   - BSRR 是原子操作，不存在读-改-写中间被中断打断的问题
 */
void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if (PinState == GPIO_PIN_SET) {
        GPIOx->BSRR = (uint32_t)GPIO_Pin;          /* 置位：写低16位 */
    } else {
        GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;   /* 复位：写高16位 */
    }
}

/**
 * @brief 翻转引脚电平
 *
 * 读取 ODR 当前值，利用 BSRR 同时完成：
 *   - 当前为高的引脚 → 写入 BR（复位）
 *   - 当前为低的引脚 → 写入 BS（置位）
 * 同样是单次写操作，不存在竞态。
 */
void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    uint32_t odr = GPIOx->ODR;
    GPIOx->BSRR = ((odr & GPIO_Pin) << 16U) | (~odr & GPIO_Pin);
}

/**
 * @brief 读取引脚输入电平
 */
GPIO_PinState GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return ((GPIOx->IDR & GPIO_Pin) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}
