#include <stdio.h>
#include "led.h"
int main(void) {
    printf("========================================\n");
    printf("  Ch5 v1: struct + me pointer\n");
    printf("========================================\n\n");
    Led_t red_led, green_led, blue_led;
    led_init(&red_led, 13);
    led_init(&green_led, 14);
    led_init(&blue_led, 15);
    led_on(&red_led);
    led_on(&green_led);
    led_on(&blue_led);
    led_set_brightness(&red_led, 80);
    led_set_brightness(&green_led, 50);
    led_set_brightness(&blue_led, 30);
    led_toggle(&red_led);
    led_toggle(&red_led);
    bool state; uint8_t brightness;
    led_get_state(&green_led, &state);
    printf("  Green LED is %s\n", state ? "ON" : "OFF");
    led_get_brightness(&blue_led, &brightness);
    printf("  Blue LED brightness = %d%%\n", brightness);
    led_deinit(&red_led);
    led_deinit(&green_led);
    led_deinit(&blue_led);
    printf("\nDone.\n");
    return 0;
}