#include <stdio.h>
#include "led.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v8: vtable (ops struct)\n\n");
    Led_t normal; PwmLed_t pwm;
    led_init(&normal,13,led_get_normal_ops());
    pwm_led_init(&pwm,5,80);
    printf("--- Polymorphism via vtable ---\n");
    led_on((Led_t*)&normal); led_on((Led_t*)&pwm);
    led_off((Led_t*)&normal); led_off((Led_t*)&pwm);
    printf("  v7: 3 fn ptrs=24B  v8: 1 ops ptr=8B per object\n");
    printf("\nDone.\n");
    return 0;
}