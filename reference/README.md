# Reference - STM32F407 官方资料库

本目录存放野火官方提供的原始学习资料。代码文件夹编号（如 `11`, `21`）对应教程中的篇章逻辑。

## 📑 教程与源码对照索引 (Quick Index)

| 章节 (PDF) | 标题内容 | 官方源码路径 (Official Code) |
| :--- | :--- | :--- |
| Chapter 07 | [新建工程—寄存器版](./pdf/chapters/Chapter_07_新建工程—寄存器版.pdf) | [7-新建工程-寄存器版本](./official_tutorial_code/7-新建工程-寄存器版本) |
| Chapter 08 | [使用寄存器点亮 LED 灯](./pdf/chapters/Chapter_08_使用寄存器点亮_LED_灯.pdf) | [8-使用寄存器点亮LED灯](./official_tutorial_code/8-使用寄存器点亮LED灯) / [三灯版](./official_tutorial_code/8-使用寄存器点亮LED灯-三个灯) |
| Chapter 09 | [自己写库—构建库函数雏形](./pdf/chapters/Chapter_09_自己写库—构建库函数雏形.pdf) | [9-自己写库—构建库函数雏形](./official_tutorial_code/9-自己写库—构建库函数雏形) |
| Chapter 11 | [新建工程—库函数版](./pdf/chapters/Chapter_11_新建工程—库函数版.pdf) | [11-新建工程-固件库版本](./official_tutorial_code/11-新建工程-固件库版本) |
| Chapter 12 | [GPIO 输出—固件库点灯](./pdf/chapters/Chapter_12_GPIO_输出—使用固件库点灯.pdf) | [12-GPIO输出—点亮LED灯](./official_tutorial_code/12-GPIO输出—使用固件库点亮LED灯) / [蜂鸣器](./official_tutorial_code/12-GPIO输出—蜂鸣器) |
| Chapter 13 | [GPIO 输入—按键检测](./pdf/chapters/Chapter_13_GPIO_输入—按键检测.pdf) | [13-GPIO输入—按键检测](./official_tutorial_code/13-GPIO输入—按键检测) |
| Chapter 14 | [GPIO—位带操作](./pdf/chapters/Chapter_14_GPIO—位带操作.pdf) | [14-位带操作](./official_tutorial_code/14-位带操作) |
| Chapter 15 | [启动文件详解](./pdf/chapters/Chapter_15_启动文件详解.pdf) | [15-启动文件详解](./official_tutorial_code/15-启动文件详解) |
| Chapter 16 | [RCC—时钟配置](./pdf/chapters/Chapter_16_RCC—使用_HSE_HSI_配置时钟.pdf) | [16-RCC—时钟配置](./official_tutorial_code/16-RCC—时钟配置（使用HSE或者HSI）) |
| Chapter 18 | [EXTI—外部中断](./pdf/chapters/Chapter_18_EXTI—外部中断_事件控制.pdf) | [18-EXTI—外部中断](./official_tutorial_code/18-EXTI—外部中断) |
| Chapter 19 | [SysTick—系统定时器](./pdf/chapters/Chapter_19_SysTick—系统定时器.pdf) | [19-SysTick—系统定时器](./official_tutorial_code/19-SysTick—系统定时器) |
| Chapter 21 | [USART—串口通讯](./pdf/chapters/Chapter_21_USART—串口通讯.pdf) | [21-USART—串口通信](./official_tutorial_code/21-USART—串口通信) |
| Chapter 22 | [DMA—直接存储区访问](./pdf/chapters/Chapter_22_DMA—直接存储区访问.pdf) | [22-DMA—直接存储区访问](./official_tutorial_code/22-DMA—直接存储区访问) |
| Chapter 24 | [I2C—读写 EEPROM](./pdf/chapters/Chapter_24_I2C—读写_EEPROM.pdf) | [24-I2C-读写EEPROM](./official_tutorial_code/24-I2C-读写EEPROM) |
| Chapter 25 | [SPI—读写串行 FLASH](./pdf/chapters/Chapter_25_SPI—读写串行_FLASH.pdf) | [25-SPI—读写串行FLASH](./official_tutorial_code/25-SPI—读写串行FLASH（W25Q128）) |
| Chapter 26 | [FatFs 文件系统](./pdf/chapters/Chapter_26_串行_FLASH_文件系统_FatFs.pdf) | [26-串行FLASH文件系统FatFs](./official_tutorial_code/26-串行FLASH文件系统FatFs) |
| Chapter 27 | [FSMC—扩展外部 SRAM](./pdf/chapters/Chapter_27_FSMC—扩展外部_SRAM.pdf) | [27-FSMC—扩展外部SRAM](./official_tutorial_code/27-FSMC—扩展外部SRAM) |
| Chapter 28 | [LCD—液晶显示](./pdf/chapters/Chapter_28_LCD—液晶显示.pdf) | [28-液晶显示](./official_tutorial_code/28-液晶显示) |
| Chapter 29 | [LCD—中英文显示](./pdf/chapters/Chapter_29_LCD—液晶显示中英文.pdf) | [29-液晶显示中英文](./official_tutorial_code/29-液晶显示中英文) |
| Chapter 30 | [电容触摸屏](./pdf/chapters/Chapter_30_电容触摸屏—触摸画板.pdf) | [30-电容触摸屏—触摸画板](./official_tutorial_code/30-电容触摸屏—触摸画板) |
| Chapter 31 | [ADC—电压采集](./pdf/chapters/Chapter_31_ADC—电压采集.pdf) | [31-ADC电压采集](./official_tutorial_code/31-ADC电压采集) |
| Chapter 32 | [TIM—基本定时器](./pdf/chapters/Chapter_32_TIM—基本定时器.pdf) | [32-TIM—基本定时器定时](./official_tutorial_code/32-TIM—基本定时器定时) |
| Chapter 33 | [TIM—高级定时器](./pdf/chapters/Chapter_33_TIM—高级定时器.pdf) | [33-TIM—高级定时器](./official_tutorial_code/33-TIM—高级定时器) / [通用定时器](./official_tutorial_code/33-TIM—通用定时器) |
| Chapter 34 | [TIM—电容按键检测](./pdf/chapters/Chapter_34_TIM—电容按键检测.pdf) | [34-TIM—电容按键](./official_tutorial_code/34-TIM—电容按键) |
| Chapter 35 | [IWDG—独立看门狗](./pdf/chapters/Chapter_35_IWDG—独立看门狗.pdf) | [35-IWDG—独立看门狗](./official_tutorial_code/35-IWDG—独立看门狗) |
| Chapter 36 | [WWDG—窗口看门狗](./pdf/chapters/Chapter_36_WWDG—窗口看门狗.pdf) | [36-WWDG—窗口看门狗](./official_tutorial_code/36-WWDG—窗口看门狗) |
| Chapter 37 | [SDIO—SD 卡读写](./pdf/chapters/Chapter_37_SDIO—SD_卡读写测试.pdf) | [37-SDIO—SD卡读写测试](./official_tutorial_code/37-SDIO—SD卡读写测试) |
| Chapter 38 | [SD 卡 FatFs](./pdf/chapters/Chapter_38_基于_SD_卡的_FatFs_文件系统.pdf) | [38-SDIO—FatFs移植与读写测试](./official_tutorial_code/38-SDIO—FatFs移植与读写测试) |
| Chapter 39 | [DAC—输出正弦波](./pdf/chapters/Chapter_39_DAC—输出正弦波.pdf) | [39-DAC—输出正弦波](./official_tutorial_code/39-DAC—输出正弦波) |
| Chapter 40 | [全彩 LED 灯](./pdf/chapters/Chapter_40_全彩_LED_灯实验.pdf) | [40-TIM—全彩LED灯](./official_tutorial_code/40-TIM—全彩LED灯) |
| Chapter 41 | [呼吸灯与 SPWM](./pdf/chapters/Chapter_41_呼吸灯与_SPWM_波.pdf) | [41-TIM—呼吸灯与SPWM波](./official_tutorial_code/41-TIM—呼吸灯与SPWM波) |
| Chapter 42 | [I2S—音频播放](./pdf/chapters/Chapter_42_I2S—音频播放与录音输入.pdf) | [42-I2S—音频](./official_tutorial_code/42-I2S—音频) |
| Chapter 43 | [ETH—以太网通信](./pdf/chapters/Chapter_43_ETH—Lwip_以太网通信.pdf) | [43-ETH—LWIP以太网](./official_tutorial_code/43-ETH—LWIP以太网) |
| Chapter 44 | [CAN—通讯实验](./pdf/chapters/Chapter_44_CAN—通讯实验.pdf) | [44-CAN通信实验](./official_tutorial_code/44-CAN通信实验) |
| Chapter 45 | [RS-485 通讯](./pdf/chapters/Chapter_45_RS-485_通讯实验.pdf) | [45-RS485通信实验](./official_tutorial_code/45-RS485通信实验) |
| Chapter 46 | [电源管理](./pdf/chapters/Chapter_46_电源管理—实现低功耗.pdf) | [46-PWR—电源管理](./official_tutorial_code/46-PWR—电源管理) |
| Chapter 47 | [RTC—实时时钟](./pdf/chapters/Chapter_47_RTC—实时时钟.pdf) | [47-RTC实时时钟](./official_tutorial_code/47-RTC实时时钟) |
| Chapter 48 | [MDK 编译过程全解](./pdf/chapters/Chapter_48_MDK_的编译过程及文件类型全解.pdf) | [48-MDK编译过程及文件全解](./official_tutorial_code/48-MDK编译过程及文件全解) |
| Chapter 49 | [在 SRAM 中调试](./pdf/chapters/Chapter_49_在_SRAM_中调试代码.pdf) | [49-RAM调试—多彩流水灯](./official_tutorial_code/49-RAM调试—多彩流水灯) |
| Chapter 50 | [读写内部 FLASH](./pdf/chapters/Chapter_50_读写内部_FLASH.pdf) | [50-读写内部FLASH](./official_tutorial_code/50-读写内部FLASH) |
| Chapter 51 | [FLASH 读写保护](./pdf/chapters/Chapter_51_设置_FLASH_的读写保护及解除.pdf) | [51-设置FLASH的读写保护与解除](./official_tutorial_code/51-设置FLASH的读写保护与解除) |
| Chapter 52 | [MPU6050 姿态检测](./pdf/chapters/Chapter_52_MPU6050_传感器—姿态检测.pdf) | [52-加速度陀螺仪—MPU6050](./official_tutorial_code/52-加速度陀螺仪—MPU6050) |
| Chapter 53 | [DCMI—OV2640 摄像头](./pdf/chapters/Chapter_53_DCMI—OV2640_摄像头.pdf) | [53-DCMI—OV2640摄像头](./official_tutorial_code/53-DCMI—OV2640摄像头) |
| Chapter 54 | [DCMI—OV5640 摄像头](./pdf/chapters/Chapter_54_DCMI—OV5640_摄像头.pdf) | [54-DCMI—OV5640摄像头](./official_tutorial_code/54-DCMI—OV5640摄像头) |
| Chapter 55 | [内核定时器控制](./pdf/chapters/Chapter_55_内核定时器控制流水灯.pdf) | [55-内核定时器控制流水灯](./official_tutorial_code/55-内核定时器控制流水灯) |

---
*Index updated by Phoebe with correct folder mapping.*
