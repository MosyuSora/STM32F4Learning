# FSMC SRAM 读写测试工程

## 功能说明

通过 FSMC 驱动外部 IS62WV51216 SRAM（1MB），演示指针直接读写和全地址校验。

## 硬件连接

本工程使用 40 个 GPIO 引脚连接外部 SRAM，全部配置为 AF12（FSMC）。

| 功能 | GPIO | 数量 |
|------|------|------|
| FSMC_A0~A18 | PF0~5, PF12~15, PG0~5, PD11~13 | 19 |
| FSMC_D0~D15 | PD14~15, PD0~1, PE7~15, PD8~10 | 16 |
| FSMC_NWE | PD5 | 1 |
| FSMC_NOE | PD4 | 1 |
| FSMC_NE4 | PG12 | 1 |
| FSMC_NBL0 | PE0 | 1 |
| FSMC_NBL1 | PE1 | 1 |

SRAM 基地址：`0x6C000000`（FSMC Bank1 NE4）

## CubeMX 配置要点

1. **FSMC**：Bank1 NE4, SRAM, 16-bit, Mode A
2. **Timing**：ADDSET=0, DATAST=8（总周期 10 HCLK = 59.5ns > 55ns）
3. **GPIO**：40 个引脚全部 AF12, 推挽, Very High Speed
4. **USART1**：115200-8N1（用于串口打印测试结果）

## 构建

本项目仅包含用户代码文件（Core/）。完整工程需要：
1. 用 CubeMX 新建 STM32F407ZGTx 工程，按上述要点配置
2. 将 Core/ 下的文件替换到 CubeMX 生成的对应位置
3. 用 Keil MDK-ARM 或 STM32CubeIDE 编译下载
