#include <stdio.h>
#include "led.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v6: Inheritance via struct nest\n\n");
    PwmLed_t pwm; RgbLed_t rgb;
    pwm_led_init(&pwm,5,80); rgb_led_init(&rgb,13,14,15);
    BaseLed_t *bp=(BaseLed_t*)&pwm; base_led_on(bp);
    bp=(BaseLed_t*)&rgb; base_led_off(bp);
    printf("&pwm=%p &pwm.base=%p (same!)\n",&pwm,&pwm.base);
    printf("\nDone.\n");
    return 0;
}