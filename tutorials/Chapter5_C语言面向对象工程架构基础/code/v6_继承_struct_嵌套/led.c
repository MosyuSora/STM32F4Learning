#include "led.h"
#include <stdio.h>
int base_led_init(BaseLed_t *base, uint8_t pin){base->pin=pin;base->is_on=false;printf("  [BaseLED] Pin%d init\n",pin);return 0;}
int base_led_on(BaseLed_t *base){base->is_on=true;printf("  [BaseLED] Pin%d -> ON\n",base->pin);return 0;}
int base_led_off(BaseLed_t *base){base->is_on=false;printf("  [BaseLED] Pin%d -> OFF\n",base->pin);return 0;}
int pwm_led_init(PwmLed_t *pwm, uint8_t pin, uint8_t duty){base_led_init(&pwm->base,pin);pwm->duty=duty;printf("  [PwmLED] Pin%d duty=%d%%\n",pin,duty);return 0;}
int pwm_led_on(PwmLed_t *pwm){base_led_on(&pwm->base);printf("  [PwmLED] brightness via duty=%d%%\n",pwm->duty);return 0;}
int pwm_led_set_duty(PwmLed_t *pwm, uint8_t duty){pwm->duty=duty;return 0;}
int rgb_led_init(RgbLed_t *rgb, uint8_t r, uint8_t g, uint8_t b){base_led_init(&rgb->base,r);rgb->r_pin=r;rgb->g_pin=g;rgb->b_pin=b;printf("  [RgbLED] R=%d G=%d B=%d\n",r,g,b);return 0;}
int rgb_led_on(RgbLed_t *rgb){base_led_on(&rgb->base);printf("  [RgbLED] R%d+G%d+B%d -> ON\n",rgb->r_pin,rgb->g_pin,rgb->b_pin);return 0;}
int rgb_led_off(RgbLed_t *rgb){base_led_off(&rgb->base);printf("  [RgbLED] R%d+G%d+B%d -> OFF\n",rgb->r_pin,rgb->g_pin,rgb->b_pin);return 0;}