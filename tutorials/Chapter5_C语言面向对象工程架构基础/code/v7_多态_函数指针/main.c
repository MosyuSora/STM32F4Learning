#include <stdio.h>

#include "led.h"

int main(void)
{
    printf("========================================\n");
    printf("  Ch5 v7: Polymorphism via fn ptr\n\n");

    Led_t normal;
    PwmLed_t pwm;

    normal_led_init(&normal, 13);
    pwm_led_init(&pwm, 5, 80);

    printf("--- Same led_on(), different behavior ---\n");
    led_on(&normal);
    led_on((Led_t *)&pwm);
    led_off(&normal);
    led_off((Led_t *)&pwm);

    printf("\nDone.\n");
    return 0;
}
