#include <stdio.h>
#include "hal_gpio.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v5: HAL Verification\n\n");
    printf("  You:  Led_t { pin, brightness, is_on }\n");
    printf("  HAL:  GPIO_TypeDef { MODER, OTYPER, ODR... }\n");
    printf("  You:  led_on(Led_t *led)\n");
    printf("  HAL:  HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, ...)\n");
    printf("  You:  led_init, led_on, led_off\n");
    printf("  HAL:  HAL_GPIO_Init, HAL_GPIO_WritePin, HAL_GPIO_TogglePin\n");
    printf("  Thousands of functions -- one pattern.\n\n");
    GPIO_InitTypeDef cfg={.Pin=GPIO_PIN_5,.Mode=GPIO_MODE_OUTPUT,.Pull=GPIO_NOPULL};
    HAL_GPIO_Init(GPIOA,&cfg);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,true);
    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,false);
    HAL_GPIO_DeInit(GPIOA,GPIO_PIN_5);
    printf("\nDone.\n");
    return 0;
}