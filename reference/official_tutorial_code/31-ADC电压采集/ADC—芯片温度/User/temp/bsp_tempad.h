#ifndef __TEMPADC_H
#define	__TEMPADC_H


#include "stm32f4xx.h"

//对于12位的ADC，3.3V的ADC值为0xfff,温度为25度时对应的电压值为0.76V即00x3AF
#define V25  0x3AF

//斜率 每摄氏度2.5mV 对应每摄氏度0x03

#define AVG_SLOPE 0x03 

void Temp_ADC1_Init(void);


#endif /* __TEMPADC_H */

