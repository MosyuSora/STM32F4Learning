/**
  ******************************************************************************
  * @file    bsp_tempad.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   获取芯片温度
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F407 开发板
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 

//温度传感器在内部和ADC1_IN16输入通道相连接）
#include "./temp/bsp_tempad.h"

//ADC1_BASE地址：0x4000 0000+0x2400 即ADC1的基地址，而他的规则数据寄存器的偏移地址是：0x4c
#define ADC1_DR_Address    ((u32)ADC1_BASE+0x4c)

__IO u16 ADC_ConvertedValue;
//__IO u16 ADC_ConvertedValueLocal;


/*
 * 函数名：ADC1_GPIO_Config
 * 描述  ：使能ADC1和DMA2的时钟
 * 输入  :无
 * 输出  ：无
 * 调用  ：内部调用
 */
static void ADC1_GPIO_Config(void)		       
{

	/* Enable DMA clock */
  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 

	/* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

 }


/* 函数名：ADC1_Mode_Config
 * 描述  ：配置ADC1的工作模式为MDA模式
 * 输入  : 无
 * 输出  ：无
 * 调用  ：内部调用
 */
static void ADC1_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	/* DMA channel1 configuration */
		   
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;		        // 外设基地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&ADC_ConvertedValue;	        // AD转换值所存放的内存基地址	（就是给个地址）
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                        // 外设作为数据传输的来源	
  DMA_InitStructure.DMA_BufferSize = 1;                                     // 定义指定DMA通道 DMA缓存的大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;          // 外设地址寄存器不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;			        // 内存地址寄存器不变
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // 数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // HalfWord
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		                    //工作在循环模式下
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;	                    //高优先级
  // 禁止DMA FIFO	，使用直连模式
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  
  // FIFO 大小，FIFO模式禁止时，这个不用配置	
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  		                        //没有设置为内存到内存的传输
	// 选择 DMA 通道，通道存在于流中
  DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
  //初始化DMA流，流相当于一个大的管道，管道里面有很多通道
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	// 使能DMA流
  DMA_Cmd(DMA2_Stream0, ENABLE);

// -------------------ADC Common 结构体 参数 初始化------------------------
	// 独立ADC模式
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  // 时钟为fpclk x分频	
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
  // 禁止DMA直接访问模式	
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  // 采样时间间隔	
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;  
  ADC_CommonInit(&ADC_CommonInitStructure);
	
  // -------------------ADC Init 结构体 参数 初始化--------------------------
  ADC_StructInit(&ADC_InitStructure);
	// ADC 分辨率
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;			                    //单通道
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;			           		//连续转换
  //禁止外部边沿触发
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1; //由软件触发启动
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;								//Right
  ADC_InitStructure.ADC_NbrOfConversion = 1;				                    //仅一个通道转换
  ADC_Init(ADC1, &ADC_InitStructure);
	

  /* ADC1 regular channel16 configuration */ 
  //设置采样通道IN16, 设置采样时间
  ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_144Cycles);	

  //使能温度传感器和内部参考电压   
  ADC_TempSensorVrefintCmd(ENABLE);                                    


  // 使能DMA请求 after last transfer (Single-ADC mode)
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	
   /* Enable ADC1 DMA */	  
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
	//开始adc转换，软件触发
	ADC_SoftwareStartConv(ADC1);
}												

/*
 * 函数名：Temp_ADC1_Init
 * 描述  ：无
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void Temp_ADC1_Init(void)
{
	ADC1_GPIO_Config();
	ADC1_Mode_Config();
}
