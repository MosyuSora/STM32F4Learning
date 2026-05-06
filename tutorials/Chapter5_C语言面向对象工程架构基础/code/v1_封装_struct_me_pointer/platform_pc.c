#include "platform.h"
#include <stdio.h>
void platform_gpio_init(uint8_t pin, uint8_t mode) {
    const char *mode_str = (mode == GPIO_MODE_OUTPUT) ? "OUTPUT" : "INPUT";
    printf("[GPIO] Pin%d init as %s\n", pin, mode_str);
}
void platform_gpio_deinit(uint8_t pin) { printf("[GPIO] Pin%d released\n", pin); }
void platform_gpio_write(uint8_t pin, bool value) {
    printf("[GPIO] Pin%d -> %s\n", pin, value ? "HIGH (ON)" : "LOW (OFF)");
}
bool platform_gpio_read(uint8_t pin) { return false; }