/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   芯片温度
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./temp/bsp_tempad.h"


// ADC1转换的电压值通过MDA方式传到SRAM
extern __IO uint16_t ADC_ConvertedValue;

/*计算后的温度值*/
 u16 Current_Temp;	 

static void Delay(__IO uint32_t nCount)	 //简单的延时函数
{
	for(; nCount != 0; nCount--);
}

//【*】注意事项：
//本实验输出的是芯片温度，一般比气温要高

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
  /*初始化USART 配置模式为 115200 8-N-1，中断接收*/
  Debug_USART_Config();
	
	Temp_ADC1_Init();
	
	printf("\r\n 这是一个内部温度传感器实验 \r\n");
	printf( "\r\n Print current Temperature  \r\n");
    while (1)
    {
	Delay(0xffffee);      // 延时 
	
	//计算方法参考《STM32数据手册》
	//计算方法1            	                 									  
 	 Current_Temp= (ADC_ConvertedValue*3.3/4096-0.76 )*1000 / 2.5+ 25 ;
	
	//计算方法2
	//Current_Temp=(ADC_ConvertedValue-V25)/AVG_SLOPE+25;	

	//10进制显示
	printf("\r\n The IC current tem= %3d ℃\r\n", Current_Temp);	      
 	 
	//16进制显示 						 
 	 //printf("\r\n The current temperature= %04x \r\n", Current_Temp);			
					
    }
}



/*********************************************END OF FILE**********************/

