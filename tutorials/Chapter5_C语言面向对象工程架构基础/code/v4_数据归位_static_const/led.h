#ifndef LED_H
#define LED_H
#include "platform.h"
typedef struct { uint8_t pin; uint8_t brightness; bool is_on; } Led_t;
int led_init(Led_t *led, uint8_t pin);
int led_deinit(Led_t *led);
int led_on(Led_t *led);
int led_off(Led_t *led);
int led_toggle(Led_t *led);
int led_set_brightness(Led_t *led, uint8_t brightness);
int led_get_state(const Led_t *led, bool *is_on, uint8_t *brightness);
int led_get_init_count(void);
void led_set_debug(int flag);
#endif