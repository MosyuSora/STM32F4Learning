/**
 * @file    main.c
 * @brief   LIB_LED 应用入口
 *
 * 和 HAL_LED/main.c 对比：
 *   - 没有 HAL_Init()，没有 SystemClock_Config()
 *   - 直接调用我们自己的 RCC_GPIO_ClkEnable() 和 GPIO_Init()
 *   - GPIO_WritePin / GPIO_TogglePin 接口名和参数与 HAL 刻意保持一致，
 *     方便以后看懂 HAL 代码时有"对照版本"
 */
#include "main.h"

/* ── 私有函数声明 ───────────────────────────────────────────────── */
static void LED_Init(void);
static void delay_ms(uint32_t ms);

/* ─────────────────────────────────────────────────────────────────
 * 主函数
 * 启动后依次点亮 红(PF6) → 绿(PF7) → 蓝(PF8)，循环流水。
 * ───────────────────────────────────────────────────────────────── */
int main(void)
{
    LED_Init();

    while (1) {
        GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);  /* 低电平，R 亮 */
        delay_ms(500);
        GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);    /* 高电平，R 灭 */

        GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);  /* 低电平，G 亮 */
        delay_ms(500);
        GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);    /* 高电平，G 灭 */

        GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);  /* 低电平，B 亮 */
        delay_ms(500);
        GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);    /* 高电平，B 灭 */
    }
}

/* ─────────────────────────────────────────────────────────────────
 * LED 引脚初始化
 * ───────────────────────────────────────────────────────────────── */
static void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. 打开 GPIOF 时钟（对应汇编里的 RCC_AHB1ENR |= (1<<5)） */
    RCC_GPIO_ClkEnable(GPIOF);

    /* 2. 默认高电平（共阳极：灯灭） */
    GPIO_WritePin(GPIOF, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_SET);

    /* 3. 配置为推挽输出，无上下拉，低速 */
    GPIO_InitStruct.Pin   = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Init(GPIOF, &GPIO_InitStruct);
}

/* ─────────────────────────────────────────────────────────────────
 * 软件延时
 * HSI 16MHz，每次循环约 4 个周期，1ms ≈ 4000 次循环。
 * 这只是粗略估算，精确延时应使用 SysTick（HAL_Delay 的做法）。
 * ───────────────────────────────────────────────────────────────── */
static void delay_ms(uint32_t ms)
{
    volatile uint32_t count = ms * 4000U;
    while (count--) {}
}

/* ─────────────────────────────────────────────────────────────────
 * 错误处理
 * ───────────────────────────────────────────────────────────────── */
void Error_Handler(void)
{
    __asm volatile ("cpsid i");  /* 关中断 */
    while (1) {}
}
