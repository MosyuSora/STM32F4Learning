#include <stdio.h>
#include "led.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v2: Information Hiding\n\n");
    Led_t red_led, green_led;
    led_init(&red_led,13); led_init(&green_led,14);
    printf("--- Correct usage ---\n"); led_on(&red_led);
    bool st; uint8_t br;
    led_get_state(&red_led,&st,&br);
    printf("  Red LED: is_on=%d, brightness=%d%%\n",st,br);
    printf("--- What coworker does (bad) ---\n");
    green_led.is_on=true;
    printf("  green_led.is_on=true (software only!)\n");
    led_get_state(&green_led,&st,&br);
    printf("  Software says: is_on=%d (hardware: nothing!)\n",st);
    led_on(&green_led);
    printf("--- Cannot call static functions ---\n");
    printf("  update_hardware() -> compile error (private!)\n");
    led_deinit(&red_led); led_deinit(&green_led);
    printf("\nDone.\n");
    return 0;
}