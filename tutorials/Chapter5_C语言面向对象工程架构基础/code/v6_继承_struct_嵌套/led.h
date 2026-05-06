#ifndef LED_H
#define LED_H
#include <stdint.h>
#include <stdbool.h>
typedef struct { uint8_t pin; bool is_on; } BaseLed_t;
int base_led_init(BaseLed_t *base, uint8_t pin);
int base_led_on(BaseLed_t *base);
int base_led_off(BaseLed_t *base);
typedef struct { BaseLed_t base; uint8_t duty; } PwmLed_t;
int pwm_led_init(PwmLed_t *pwm, uint8_t pin, uint8_t duty);
int pwm_led_on(PwmLed_t *pwm);
int pwm_led_set_duty(PwmLed_t *pwm, uint8_t duty);
typedef struct { BaseLed_t base; uint8_t r_pin; uint8_t g_pin; uint8_t b_pin; } RgbLed_t;
int rgb_led_init(RgbLed_t *rgb, uint8_t r, uint8_t g, uint8_t b);
int rgb_led_on(RgbLed_t *rgb);
int rgb_led_off(RgbLed_t *rgb);
#endif