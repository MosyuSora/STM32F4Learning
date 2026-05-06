#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdint.h>
#include <stdbool.h>
#define GPIO_MODE_OUTPUT 0
#define GPIO_MODE_INPUT 1
void platform_gpio_init(uint8_t pin, uint8_t mode);
void platform_gpio_deinit(uint8_t pin);
void platform_gpio_write(uint8_t pin, bool value);
bool platform_gpio_read(uint8_t pin);
void platform_pwm_init(uint8_t pin);
void platform_pwm_set_duty(uint8_t pin, uint8_t duty);
#endif