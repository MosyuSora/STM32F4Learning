#ifndef LED_H
#define LED_H
#include <stdint.h>
#include <stdbool.h>
struct Led_t;
typedef struct { void (*on)(struct Led_t *led); void (*off)(struct Led_t *led); void (*toggle)(struct Led_t *led); } LedOps_t;
typedef struct Led_t { uint8_t pin; bool is_on; const LedOps_t *ops; } Led_t;
int led_init(Led_t *led, uint8_t pin, const LedOps_t *ops);
int led_on(Led_t *led);
int led_off(Led_t *led);
int led_toggle(Led_t *led);
typedef struct { Led_t base; uint8_t duty; } PwmLed_t;
int pwm_led_init(PwmLed_t *pwm, uint8_t pin, uint8_t duty);
const LedOps_t *led_get_normal_ops(void);
#endif