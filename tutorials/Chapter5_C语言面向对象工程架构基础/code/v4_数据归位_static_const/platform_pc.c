#include "platform.h"
#include <stdio.h>
void platform_gpio_init(uint8_t pin, uint8_t mode){const char*m=(mode==GPIO_MODE_OUTPUT)?"OUTPUT":"INPUT";printf("[GPIO] Pin%d init as %s\n",pin,m);}
void platform_gpio_deinit(uint8_t pin){printf("[GPIO] Pin%d released\n",pin);}
void platform_gpio_write(uint8_t pin, bool value){printf("[GPIO] Pin%d -> %s\n",pin,value?"HIGH":"LOW");}
bool platform_gpio_read(uint8_t pin){return false;}