/**
 * @file    rcc.c
 * @brief   RCC 时钟使能实现
 *
 * 通过外设指针匹配 AHB1ENR 对应的 bit，
 * 一个函数就能处理所有 GPIOA..GPIOI 的时钟使能，
 * 避免了标准库里每个端口一个函数的重复写法。
 */
#include "rcc.h"

/**
 * @brief 使能指定 GPIO 端口的 AHB1 时钟
 * @param GPIOx  目标端口指针，如 GPIOA、GPIOF 等
 *
 * 实现原理：
 *   每个 GPIO 端口的基地址相差固定的 0x400，
 *   用指针减法可以算出相对于 GPIOA 的偏移，
 *   再右移 10 位（即除以 0x400）得到端口编号，
 *   也就是 AHB1ENR 里对应的 bit 位置。
 */
void RCC_GPIO_ClkEnable(GPIO_TypeDef *GPIOx)
{
    /* GPIOA 在 AHB1ENR 的 bit0，GPIOB 在 bit1，以此类推 */
    uint32_t bit = ((uint32_t)GPIOx - GPIOA_BASE) >> 10U;
    RCC->AHB1ENR |= (1UL << bit);
}
