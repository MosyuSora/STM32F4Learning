#ifndef MOTOR_H
#define MOTOR_H
#include "platform.h"
typedef struct { uint8_t pin; uint8_t speed; bool is_running; } Motor_t;
int motor_init(Motor_t *motor, uint8_t pwm_pin);
int motor_deinit(Motor_t *motor);
int motor_start(Motor_t *motor);
int motor_stop(Motor_t *motor);
int motor_set_speed(Motor_t *motor, uint8_t speed_percent);
int motor_get_speed(const Motor_t *motor, uint8_t *speed);
#endif