/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   串口接发测试，串口接收到数据后马上回传。
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
#include "test.h"

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
  /*初始化USART 配置模式为 115200 8-N-1，中断接收*/
  Debug_USART_Config();
	
	printf("欢迎使用野火STM32开发板\n\n\n\n");
	printf("使用空闲中断接收不定长字符串简易演示\n\n\n\n");
	
  while(1)
	{	
		
		if(test_data.flag == 1)
		{			
			do_process(&test_data);							
		}
		
	}	
}



/*********************************************END OF FILE**********************/

