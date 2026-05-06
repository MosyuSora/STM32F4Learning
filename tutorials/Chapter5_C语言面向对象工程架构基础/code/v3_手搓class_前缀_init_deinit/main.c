#include <stdio.h>
#include "led.h"
#include "motor.h"
int main(void){
    printf("========================================\n");
    printf("  Ch5 v3: You built a class in C\n\n");
    Led_t red_led; Motor_t fan_motor;
    led_init(&red_led,5); motor_init(&fan_motor,8);
    printf("--- Operate LED ---\n"); led_on(&red_led); led_set_brightness(&red_led,80); led_toggle(&red_led); led_toggle(&red_led);
    printf("--- Operate Motor ---\n"); motor_set_speed(&fan_motor,60); motor_start(&fan_motor); motor_set_speed(&fan_motor,90); motor_stop(&fan_motor);
    printf("  LED class: led_init, led_on -- Motor class: motor_init, motor_start\n");
    printf("  Prefix IS the class name.\n");
    led_deinit(&red_led); motor_deinit(&fan_motor);
    printf("\nDone.\n");
    return 0;
}