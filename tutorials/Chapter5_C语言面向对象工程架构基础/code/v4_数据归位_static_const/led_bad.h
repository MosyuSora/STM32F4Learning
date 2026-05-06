#ifndef LED_BAD_H
#define LED_BAD_H
#include "platform.h"
int bad_led_init(uint8_t pin);
int bad_led_on(void);
int bad_led_off(void);
int bad_led_toggle(void);
int bad_led_set_brightness(uint8_t brightness);
int bad_led_get_brightness(uint8_t *brightness);
#endif