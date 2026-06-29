#ifndef LED_DEMO_H
#define LED_DEMO_H

#include <stdint.h>
#include <stdbool.h>
#include "container_of.h"

typedef struct Led Led_t;

typedef struct {
    void (*on)(Led_t *led);
} LedOps_t;

struct Led {
    const char *name;
    bool is_on;
    const LedOps_t *ops;
};

typedef struct {
    Led_t base;
    uint8_t duty;
} PwmLed_t;

#define PWM_LED_FROM_BASE(ptr) container_of(ptr, PwmLed_t, base)

typedef struct {
    Led_t base;
    uint8_t addr;
    uint8_t output_reg;
} I2cLed_t;

#define I2C_LED_FROM_BASE(ptr) container_of(ptr, I2cLed_t, base)

void pwm_led_init(PwmLed_t *pwm, const char *name, uint8_t duty);
void i2c_led_init(I2cLed_t *i2c, const char *name, uint8_t addr, uint8_t output_reg);
void led_on(Led_t *led);
void led_container_of_demo(void);

#endif
