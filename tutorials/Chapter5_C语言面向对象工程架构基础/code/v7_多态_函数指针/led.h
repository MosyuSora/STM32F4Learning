#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

struct Led_t;

typedef struct Led_t {
    uint8_t pin;
    bool is_on;
    void (*do_on)(struct Led_t *led);
    void (*do_off)(struct Led_t *led);
    void (*do_toggle)(struct Led_t *led);
} Led_t;

int led_init(Led_t *led,
             uint8_t pin,
             void (*on)(Led_t *led),
             void (*off)(Led_t *led),
             void (*toggle)(Led_t *led));
int normal_led_init(Led_t *led, uint8_t pin);
int led_on(Led_t *led);
int led_off(Led_t *led);
int led_toggle(Led_t *led);

typedef struct {
    Led_t base;
    uint8_t duty;
} PwmLed_t;

int pwm_led_init(PwmLed_t *pwm, uint8_t pin, uint8_t duty);

#endif
