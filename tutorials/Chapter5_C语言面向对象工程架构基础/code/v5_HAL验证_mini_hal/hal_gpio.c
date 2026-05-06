#include "hal_gpio.h"
#include <stdio.h>
#include <string.h>
static GPIO_TypeDef g_gpioa_regs={0};
static GPIO_TypeDef g_gpiob_regs={0};
static GPIO_TypeDef g_gpioc_regs={0};
#define GPIOA (&g_gpioa_regs)
#define GPIOB (&g_gpiob_regs)
#define GPIOC (&g_gpioc_regs)
static uint8_t get_pin_number(uint16_t mask){for(uint8_t i=0;i<16;i++)if(mask&(1u<<i))return i;return 0;}
static const char* port_name(GPIO_TypeDef *GPIOx){if(GPIOx==&g_gpioa_regs)return"A";if(GPIOx==&g_gpiob_regs)return"B";if(GPIOx==&g_gpioc_regs)return"C";return"?";}
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init){if(GPIOx==NULL||init==NULL)return;uint8_t p=get_pin_number(init->Pin);uint32_t m=0x3u<<(p*2);GPIOx->MODER&=~m;GPIOx->MODER|=(init->Mode&0x3u)<<(p*2);uint32_t pm=0x3u<<(p*2);GPIOx->PUPDR&=~pm;GPIOx->PUPDR|=(init->Pull&0x3u)<<(p*2);printf("[HAL] GPIO%c Pin%d init\n",port_name(GPIOx)[0],p);}
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin){if(GPIOx==NULL)return;uint8_t p=get_pin_number(pin);uint32_t m=0x3u<<(p*2);GPIOx->MODER&=~m;printf("[HAL] GPIO%c Pin%d deinit\n",port_name(GPIOx)[0],p);}
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t pin, bool value){if(GPIOx==NULL)return;uint8_t p=get_pin_number(pin);if(value)GPIOx->BSRR=(uint32_t)pin;else GPIOx->BSRR=(uint32_t)pin<<16;printf("[HAL] GPIO%c Pin%d -> %s\n",port_name(GPIOx)[0],p,value?"HIGH":"LOW");}
bool HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t pin){if(GPIOx==NULL)return false;return(GPIOx->IDR&pin)!=0;}
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin){if(GPIOx==NULL)return;uint8_t p=get_pin_number(pin);bool cur=(GPIOx->ODR&pin)!=0;HAL_GPIO_WritePin(GPIOx,pin,!cur);}