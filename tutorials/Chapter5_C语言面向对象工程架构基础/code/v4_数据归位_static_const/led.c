#include "led.h"
#include <stdio.h>
static const uint8_t MAX_BRIGHTNESS=100;
static const uint8_t MAX_PIN=15;
static int s_init_count=0;
static int s_debug_flag=0;
static void update_hardware(Led_t *led){bool target=led->is_on;platform_gpio_write(led->pin,target);if(s_debug_flag)printf("  [DEBUG] led.c: pin=%d state=%d\n",led->pin,target);}
static bool is_brightness_valid(uint8_t b){return b<=MAX_BRIGHTNESS;}
static bool is_pin_valid(uint8_t p){return p<=MAX_PIN;}
int led_init(Led_t *led, uint8_t pin){if(led==NULL||!is_pin_valid(pin))return -1;led->pin=pin;led->brightness=0;led->is_on=false;platform_gpio_init(pin,GPIO_MODE_OUTPUT);update_hardware(led);s_init_count++;printf("  [LED] Pin%d initialized (total inits: %d)\n",pin,s_init_count);return 0;}
int led_deinit(Led_t *led){if(led==NULL)return -1;platform_gpio_write(led->pin,false);platform_gpio_deinit(led->pin);led->pin=0;led->brightness=0;led->is_on=false;return 0;}
int led_on(Led_t *led){if(led==NULL)return -1;led->is_on=true;update_hardware(led);return 0;}
int led_off(Led_t *led){if(led==NULL)return -1;led->is_on=false;update_hardware(led);return 0;}
int led_toggle(Led_t *led){if(led==NULL)return -1;return led->is_on?led_off(led):led_on(led);}
int led_set_brightness(Led_t *led, uint8_t b){if(led==NULL||!is_brightness_valid(b))return -1;led->brightness=b;if(led->is_on)update_hardware(led);return 0;}
int led_get_state(const Led_t *led, bool *is_on, uint8_t *brightness){if(led==NULL||is_on==NULL||brightness==NULL)return -1;*is_on=led->is_on;*brightness=led->brightness;return 0;}
int led_get_init_count(void){return s_init_count;}
void led_set_debug(int flag){s_debug_flag=flag;}