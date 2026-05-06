#include "led_bad.h"
#include <stdio.h>
int g_pin=0;int g_brightness=0;int g_is_on=0;int g_init_count=0;
#define MAX_BRIGHTNESS 255
int bad_led_init(uint8_t pin){g_pin=pin;g_brightness=0;g_is_on=0;platform_gpio_init(pin,GPIO_MODE_OUTPUT);platform_gpio_write(pin,false);g_init_count++;printf("  [LED_BAD] Pin%d initialized\n",pin);return 0;}
int bad_led_on(void){g_is_on=1;platform_gpio_write(g_pin,true);printf("  [LED_BAD] Pin%d -> ON\n",g_pin);return 0;}
int bad_led_off(void){g_is_on=0;platform_gpio_write(g_pin,false);printf("  [LED_BAD] Pin%d -> OFF\n",g_pin);return 0;}
int bad_led_toggle(void){return g_is_on?bad_led_off():bad_led_on();}
int bad_led_set_brightness(uint8_t b){if(b>MAX_BRIGHTNESS)return -1;g_brightness=b;return 0;}
int bad_led_get_brightness(uint8_t *brightness){if(brightness==NULL)return -1;*brightness=g_brightness;return 0;}