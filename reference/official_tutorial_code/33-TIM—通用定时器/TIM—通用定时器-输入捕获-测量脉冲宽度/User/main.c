/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   高级控制定时器测量脉宽
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
#include "./GeneralTim/bsp_GeneralTim.h"  

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void) 
{
	uint32_t time;
	
	// TIM 计数器的驱动时钟 TIM5在低速APB1上
	uint32_t TIM_PscCLK = 84000000 / (GENERAL_TIM_PSC+1);
	
	/* 串口初始化 */
  Debug_USART_Config();
	/* 定时器初始化 */
	GENERAL_TIM_Init();
	
	printf ( "\r\n野火 STM32 输入捕获实验\r\n" );
	printf ( "\r\n按下K1，测试K1按下的时间\r\n" );
	
	while ( 1 )
	{
		if(TIM_ICUserValueStructure.Capture_FinishFlag == 1)
		{
			// 计算高电平时间的计数器的值
			time = TIM_ICUserValueStructure.Capture_Period * (GENERAL_TIM_PERIOD+1) + 
			       (TIM_ICUserValueStructure.Capture_CcrValue+1);
			
			// 打印高电平脉宽时间
			printf ( "\r\n测得高电平脉宽时间：%d.%d s\r\n",time/TIM_PscCLK,time%TIM_PscCLK );
			
			TIM_ICUserValueStructure.Capture_FinishFlag = 0;			
		}		
	}
}



/*********************************************END OF FILE**********************/

