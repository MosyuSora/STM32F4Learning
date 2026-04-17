/**
  ******************************************************************************
  * @file    bsp_sram.c
  * @brief   外部 SRAM (IS62WV51216) 驱动
  *
  *  FSMC 配置要点：
  *    - Bank1 NE4, 基地址 0x6C000000
  *    - 16 位数据宽度, 模式 A
  *    - ADDSET=0, DATAST=8  →  总周期 10 HCLK = 59.5ns > 55ns
  *    - 40 个 GPIO 全部 AF12, 推挽, Very High Speed
  ******************************************************************************
  */
#include "bsp_sram.h"

static SRAM_HandleTypeDef hsram;

/* ======================================================================
 *  GPIO 初始化 — 40 个引脚
 * ====================================================================== */
static void BSP_SRAM_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能所有相关 GPIO 端口时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* 公共配置 */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;

    /* ── 端口 D ── */
    /* D2=PD0, D3=PD1, NWE=PD5, NOE=PD4, D13=PD8, D14=PD9, D15=PD10,
       A16=PD11, A17=PD12, A18=PD13, D0=PD14, D1=PD15 */
    GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4  | GPIO_PIN_5
                        | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11
                        | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* ── 端口 E ── */
    /* NBL0=PE0, NBL1=PE1, D4=PE7~D12=PE15 */
    GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7  | GPIO_PIN_8
                        | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
                        | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* ── 端口 F ── */
    /* A0=PF0~A5=PF5, A6=PF12~A9=PF15 */
    GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3
                        | GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_12 | GPIO_PIN_13
                        | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* ── 端口 G ── */
    /* A10=PG0~A15=PG5, NE4=PG12 */
    GPIO_InitStruct.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3
                        | GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

/* ======================================================================
 *  FSMC 初始化
 * ====================================================================== */
void BSP_SRAM_Init(void)
{
    FSMC_NORSRAM_TimingTypeDef timing = {0};

    /* 使能 FSMC 时钟 */
    __HAL_RCC_FSMC_CLK_ENABLE();

    /* GPIO */
    BSP_SRAM_GPIO_Init();

    /* ── SRAM handle 配置 ── */
    hsram.Instance  = FSMC_NORSRAM_DEVICE;
    hsram.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;

    hsram.Init.NSBank             = FSMC_NORSRAM_BANK4;           /* NE4 */
    hsram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;
    hsram.Init.PageSize           = FSMC_PAGE_SIZE_NONE;

    /* ── 时序 — 模式 A ── */
    timing.AddressSetupTime      = 0;   /* (0+1) HCLK = 5.95ns  */
    timing.AddressHoldTime       = 0;   /* 模式 A 不用            */
    timing.DataSetupTime         = 8;   /* (8+1) HCLK = 53.6ns  */
    timing.BusTurnAroundDuration = 0;
    timing.CLKDivision           = 0;
    timing.DataLatency           = 0;
    timing.AccessMode            = FSMC_ACCESS_MODE_A;
    /* 读/写总周期 = (0+1+8+1) × 5.95ns = 59.5ns ≥ 55ns ✓ */

    HAL_SRAM_Init(&hsram, &timing, NULL);
}

/* ======================================================================
 *  便利读写函数
 * ====================================================================== */
void SRAM_WriteHalfWord(uint32_t offset, uint16_t data)
{
    *((__IO uint16_t *)(SRAM_BASE_ADDR + offset)) = data;
}

uint16_t SRAM_ReadHalfWord(uint32_t offset)
{
    return *((__IO uint16_t *)(SRAM_BASE_ADDR + offset));
}

void SRAM_WriteBuffer(uint32_t offset, uint16_t *buf, uint32_t count)
{
    __IO uint16_t *p = (__IO uint16_t *)(SRAM_BASE_ADDR + offset);
    for (uint32_t i = 0; i < count; i++)
        p[i] = buf[i];
}

void SRAM_ReadBuffer(uint32_t offset, uint16_t *buf, uint32_t count)
{
    __IO uint16_t *p = (__IO uint16_t *)(SRAM_BASE_ADDR + offset);
    for (uint32_t i = 0; i < count; i++)
        buf[i] = p[i];
}

/* ======================================================================
 *  全地址校验测试
 *  返回 0 = 全部通过, 非零 = 出错的字节偏移地址
 * ====================================================================== */
uint32_t BSP_SRAM_Test(void)
{
    uint32_t i;
    uint16_t *pSRAM = (uint16_t *)SRAM_BASE_ADDR;
    uint32_t half_word_count = SRAM_SIZE / 2;  /* 512K 个半字 */

    /* ── 写入：每个地址写入自己的地址低 16 位 ── */
    for (i = 0; i < half_word_count; i++)
    {
        pSRAM[i] = (uint16_t)(i & 0xFFFF);
    }

    /* ── 读回校验 ── */
    for (i = 0; i < half_word_count; i++)
    {
        if (pSRAM[i] != (uint16_t)(i & 0xFFFF))
        {
            return i * 2;  /* 返回出错的字节偏移 */
        }
    }

    return 0;  /* 全部通过 */
}
