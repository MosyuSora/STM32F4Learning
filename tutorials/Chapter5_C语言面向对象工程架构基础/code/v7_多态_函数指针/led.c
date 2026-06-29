#include "led.h"

#include <stdio.h>

static void normal_on(Led_t *led)
{
    led->is_on = true;
    printf("  [Normal] Pin%d -> ON\n", led->pin);
}

static void normal_off(Led_t *led)
{
    led->is_on = false;
    printf("  [Normal] Pin%d -> OFF\n", led->pin);
}

static void normal_toggle(Led_t *led)
{
    if (led->is_on) {
        normal_off(led);
    } else {
        normal_on(led);
    }
}

static void pwm_on(Led_t *led)
{
    PwmLed_t *real = (PwmLed_t *)led;

    real->base.is_on = true;
    printf("  [PwmLED] Pin%d -> ON, duty=%d%%\n",
           real->base.pin,
           real->duty);
}

static void pwm_off(Led_t *led)
{
    PwmLed_t *real = (PwmLed_t *)led;

    real->base.is_on = false;
    printf("  [PwmLED] Pin%d -> OFF\n", real->base.pin);
}

static void pwm_toggle(Led_t *led)
{
    if (led->is_on) {
        pwm_off(led);
    } else {
        pwm_on(led);
    }
}

int led_init(Led_t *led,
             uint8_t pin,
             void (*on)(Led_t *led),
             void (*off)(Led_t *led),
             void (*toggle)(Led_t *led))
{
    led->pin = pin;
    led->is_on = false;
    led->do_on = on;
    led->do_off = off;
    led->do_toggle = toggle;
    printf("  [Led] Pin%d init polymorphic\n", pin);
    return 0;
}

int normal_led_init(Led_t *led, uint8_t pin)
{
    return led_init(led, pin, normal_on, normal_off, normal_toggle);
}

int led_on(Led_t *led)
{
    led->do_on(led);
    return 0;
}

int led_off(Led_t *led)
{
    led->do_off(led);
    return 0;
}

int led_toggle(Led_t *led)
{
    led->do_toggle(led);
    return 0;
}

int pwm_led_init(PwmLed_t *pwm, uint8_t pin, uint8_t duty)
{
    led_init(&pwm->base, pin, pwm_on, pwm_off, pwm_toggle);
    pwm->duty = duty;
    return 0;
}
