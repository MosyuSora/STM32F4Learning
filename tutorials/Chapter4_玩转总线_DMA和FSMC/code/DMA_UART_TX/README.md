# DMA UART TX 示例工程

## 功能说明

通过 DMA2 Stream7 Channel4 将 RAM 中的字符串经 USART1 发送出去，演示 DMA 非阻塞发送。

## 硬件连接

| 功能 | 引脚 | 说明 |
|------|------|------|
| USART1_TX | PA9 | 连接 USB-TTL 的 RX |
| USART1_RX | PA10 | 连接 USB-TTL 的 TX |
| LED | PF10 | 发送完成翻转（可选） |

## CubeMX 配置要点

1. **USART1**：Asynchronous, 115200-8N1, PA9/PA10
2. **DMA**：USART1_TX → DMA2 Stream7, Memory To Peripheral, Byte, Normal, Memory Inc Enable
3. **NVIC**：使能 DMA2 Stream7 Global Interrupt
4. **时钟**：HSE → PLL → SYSCLK 168MHz

## 构建

本项目仅包含用户代码文件（Core/）。完整工程需要：
1. 用 CubeMX 新建 STM32F407ZGTx 工程，按上述要点配置
2. 将 Core/ 下的文件替换到 CubeMX 生成的对应位置
3. 用 Keil MDK-ARM 或 STM32CubeIDE 编译下载
