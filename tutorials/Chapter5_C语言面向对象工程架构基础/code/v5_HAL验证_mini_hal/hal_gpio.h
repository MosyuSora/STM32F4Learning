#ifndef HAL_GPIO_H
#define HAL_GPIO_H
#include "gpio_typedef.h"
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *init);
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t pin, bool value);
bool HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t pin);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t pin);
#endif