/**
 * @file    stm32f4xx.h
 * @brief   STM32F407 最小寄存器定义
 *
 * 只保留本工程实际用到的外设：RCC 和 GPIOF。
 * 对比 ST 官方 CMSIS 头，这里把数百行精简到了最小可工作集合，
 * 目的是让读者看清楚"寄存器结构体"到底封装了什么。
 */
#ifndef __STM32F4XX_H
#define __STM32F4XX_H

#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════════
 * 1. GPIO 寄存器结构体
 *    每个成员对应数据手册 GPIO 寄存器图里的一个寄存器，
 *    按偏移地址顺序排列，编译器会把它们紧密打包。
 * ═══════════════════════════════════════════════════════════════════ */
typedef struct {
    volatile uint32_t MODER;    /*!< 偏移 0x00  引脚模式寄存器        */
    volatile uint32_t OTYPER;   /*!< 偏移 0x04  输出类型寄存器        */
    volatile uint32_t OSPEEDR;  /*!< 偏移 0x08  输出速度寄存器        */
    volatile uint32_t PUPDR;    /*!< 偏移 0x0C  上下拉寄存器          */
    volatile uint32_t IDR;      /*!< 偏移 0x10  输入数据寄存器        */
    volatile uint32_t ODR;      /*!< 偏移 0x14  输出数据寄存器        */
    volatile uint32_t BSRR;     /*!< 偏移 0x18  位置位/复位寄存器     */
    volatile uint32_t LCKR;     /*!< 偏移 0x1C  配置锁定寄存器        */
    volatile uint32_t AFR[2];   /*!< 偏移 0x20  复用功能寄存器 [0..1] */
} GPIO_TypeDef;

/* ═══════════════════════════════════════════════════════════════════
 * 2. RCC 寄存器结构体（仅列出本工程需要的寄存器）
 *    中间保留字段用 uint32_t RESERVED 占位，保证偏移正确。
 * ═══════════════════════════════════════════════════════════════════ */
typedef struct {
    volatile uint32_t CR;           /*!< 偏移 0x00  时钟控制寄存器            */
    volatile uint32_t PLLCFGR;      /*!< 偏移 0x04  PLL 配置寄存器            */
    volatile uint32_t CFGR;         /*!< 偏移 0x08  时钟配置寄存器            */
    volatile uint32_t CIR;          /*!< 偏移 0x0C  时钟中断寄存器            */
    volatile uint32_t AHB1RSTR;     /*!< 偏移 0x10  AHB1 外设复位寄存器       */
    volatile uint32_t AHB2RSTR;     /*!< 偏移 0x14  AHB2 外设复位寄存器       */
    volatile uint32_t AHB3RSTR;     /*!< 偏移 0x18  AHB3 外设复位寄存器       */
    uint32_t          RESERVED0;    /*!< 偏移 0x1C  保留                      */
    volatile uint32_t APB1RSTR;     /*!< 偏移 0x20  APB1 外设复位寄存器       */
    volatile uint32_t APB2RSTR;     /*!< 偏移 0x24  APB2 外设复位寄存器       */
    uint32_t          RESERVED1[2]; /*!< 偏移 0x28-0x2C 保留                  */
    volatile uint32_t AHB1ENR;      /*!< 偏移 0x30  AHB1 外设时钟使能寄存器   */
    volatile uint32_t AHB2ENR;      /*!< 偏移 0x34  AHB2 外设时钟使能寄存器   */
    volatile uint32_t AHB3ENR;      /*!< 偏移 0x38  AHB3 外设时钟使能寄存器   */
    uint32_t          RESERVED2;    /*!< 偏移 0x3C  保留                      */
    volatile uint32_t APB1ENR;      /*!< 偏移 0x40  APB1 外设时钟使能寄存器   */
    volatile uint32_t APB2ENR;      /*!< 偏移 0x44  APB2 外设时钟使能寄存器   */
} RCC_TypeDef;

/* ═══════════════════════════════════════════════════════════════════
 * 3. 外设基地址
 *    来自参考手册 Table 1. Register boundary addresses
 * ═══════════════════════════════════════════════════════════════════ */
#define PERIPH_BASE         0x40000000UL
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)

#define GPIOA_BASE          (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE          (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE          (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE          (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE          (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOF_BASE          (AHB1PERIPH_BASE + 0x1400UL)
#define GPIOG_BASE          (AHB1PERIPH_BASE + 0x1800UL)
#define GPIOH_BASE          (AHB1PERIPH_BASE + 0x1C00UL)
#define GPIOI_BASE          (AHB1PERIPH_BASE + 0x2000UL)
#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800UL)

/* ═══════════════════════════════════════════════════════════════════
 * 4. 外设指针
 *    把基地址强转成对应结构体指针，这是 HAL/标准库的核心技巧。
 *    之后写 GPIOF->MODER 等同于直接写地址 0x40021400。
 * ═══════════════════════════════════════════════════════════════════ */
#define GPIOA   ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF   ((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG   ((GPIO_TypeDef *) GPIOG_BASE)
#define GPIOH   ((GPIO_TypeDef *) GPIOH_BASE)
#define GPIOI   ((GPIO_TypeDef *) GPIOI_BASE)
#define RCC     ((RCC_TypeDef  *) RCC_BASE)

/* ═══════════════════════════════════════════════════════════════════
 * 5. 常用位定义
 * ═══════════════════════════════════════════════════════════════════ */
#define RCC_AHB1ENR_GPIOAEN  (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN  (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN  (1UL << 2)
#define RCC_AHB1ENR_GPIODEN  (1UL << 3)
#define RCC_AHB1ENR_GPIOEEN  (1UL << 4)
#define RCC_AHB1ENR_GPIOFEN  (1UL << 5)
#define RCC_AHB1ENR_GPIOGEN  (1UL << 6)
#define RCC_AHB1ENR_GPIOHEN  (1UL << 7)
#define RCC_AHB1ENR_GPIOIEN  (1UL << 8)

#endif /* __STM32F4XX_H */
