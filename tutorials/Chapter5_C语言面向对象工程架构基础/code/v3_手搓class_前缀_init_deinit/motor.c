#include "motor.h"
#include <stdio.h>
static void update_hardware(Motor_t *motor){if(motor->is_running){platform_pwm_set_duty(motor->pin,motor->speed);}}
static bool is_speed_valid(uint8_t s){return s<=100;}
int motor_init(Motor_t *motor, uint8_t pwm_pin){if(motor==NULL)return -1;motor->pin=pwm_pin;motor->speed=0;motor->is_running=false;platform_pwm_init(pwm_pin);platform_pwm_set_duty(pwm_pin,0);printf("  [Motor] Pin%d initialized\n",pwm_pin);return 0;}
int motor_deinit(Motor_t *motor){if(motor==NULL)return -1;platform_pwm_set_duty(motor->pin,0);platform_gpio_deinit(motor->pin);motor->pin=0;motor->speed=0;motor->is_running=false;return 0;}
int motor_start(Motor_t *motor){if(motor==NULL)return -1;motor->is_running=true;update_hardware(motor);printf("  [Motor] Pin%d -> START (speed=%d%%)\n",motor->pin,motor->speed);return 0;}
int motor_stop(Motor_t *motor){if(motor==NULL)return -1;motor->is_running=false;platform_pwm_set_duty(motor->pin,0);printf("  [Motor] Pin%d -> STOP\n",motor->pin);return 0;}
int motor_set_speed(Motor_t *motor, uint8_t sp){if(motor==NULL||!is_speed_valid(sp))return -1;motor->speed=sp;if(motor->is_running)update_hardware(motor);return 0;}
int motor_get_speed(const Motor_t *motor, uint8_t *speed){if(motor==NULL||speed==NULL)return -1;*speed=motor->speed;return 0;}