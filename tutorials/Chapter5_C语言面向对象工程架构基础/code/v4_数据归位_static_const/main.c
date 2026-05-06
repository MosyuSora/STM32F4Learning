#include <stdio.h>
#include "led.h"
#include "led_bad.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v4: Data Ownership\n\n");
    Led_t red_led, green_led;
    led_init(&red_led,13); led_init(&green_led,14);
    led_on(&red_led); led_on(&green_led); led_toggle(&red_led);
    printf("  Init count: %d\n",led_get_init_count());
    led_deinit(&red_led); led_deinit(&green_led);
    printf("--- Bad version ---\n");
    bad_led_init(13); bad_led_init(14);
    bad_led_on();
    printf("  g_init_count=%d (someone changed to 999)\n",g_init_count);
    g_init_count=999;
    printf("  g_init_count=%d (modified externally!)\n",g_init_count);
    printf("\nDone.\n");
    return 0;
}