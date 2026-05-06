#include "led.h"
#include <stdio.h>
static void update_hardware(Led_t *led){ platform_gpio_write(led->pin,led->is_on); }
static bool is_brightness_valid(uint8_t b){ return b<=100; }
int led_init(Led_t *led, uint8_t pin){ if(led==NULL)return -1; led->pin=pin;led->brightness=0;led->is_on=false; platform_gpio_init(pin,GPIO_MODE_OUTPUT); update_hardware(led); printf("  [LED] Pin%d LED initialized\n",pin); return 0; }
int led_deinit(Led_t *led){ if(led==NULL)return -1; platform_gpio_write(led->pin,false); platform_gpio_deinit(led->pin); led->pin=0;led->brightness=0;led->is_on=false; return 0; }
int led_on(Led_t *led){ if(led==NULL)return -1; led->is_on=true; update_hardware(led); printf("  [LED] Pin%d -> ON\n",led->pin); return 0; }
int led_off(Led_t *led){ if(led==NULL)return -1; led->is_on=false; update_hardware(led); printf("  [LED] Pin%d -> OFF\n",led->pin); return 0; }
int led_toggle(Led_t *led){ if(led==NULL)return -1; return led->is_on?led_off(led):led_on(led); }
int led_set_brightness(Led_t *led, uint8_t b){ if(led==NULL||!is_brightness_valid(b))return -1; led->brightness=b; if(led->is_on)printf("  [LED] Pin%d brightness -> %d%%\n",led->pin,b); return 0; }
int led_get_state(const Led_t *led, bool *is_on, uint8_t *brightness){ if(led==NULL||is_on==NULL||brightness==NULL)return -1; *is_on=led->is_on; *brightness=led->brightness; return 0; }