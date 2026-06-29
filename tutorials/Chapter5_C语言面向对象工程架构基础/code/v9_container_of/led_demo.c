#include "led_demo.h"
#include <stddef.h>
#include <stdio.h>

static void pwm_led_on(Led_t *led)
{
    PwmLed_t *pwm = PWM_LED_FROM_BASE(led);

    pwm->base.is_on = true;
    printf("  [LED] %s on, duty=%u\n", pwm->base.name, pwm->duty);
    printf("        Led_t base at %p -> PwmLed_t at %p\n",
           (void *)led,
           (void *)pwm);
}

static const LedOps_t pwm_led_ops = {
    .on = pwm_led_on,
};

static void i2c_led_on(Led_t *led)
{
    I2cLed_t *i2c = I2C_LED_FROM_BASE(led);

    i2c->base.is_on = true;
    printf("  [LED] %s on, addr=0x%02X, output_reg=0x%02X\n",
           i2c->base.name,
           i2c->addr,
           i2c->output_reg);
    printf("        Led_t base at %p -> I2cLed_t at %p\n",
           (void *)led,
           (void *)i2c);
}

static const LedOps_t i2c_led_ops = {
    .on = i2c_led_on,
};

void pwm_led_init(PwmLed_t *pwm, const char *name, uint8_t duty)
{
    pwm->base.name = name;
    pwm->base.is_on = false;
    pwm->base.ops = &pwm_led_ops;
    pwm->duty = duty;
}

void i2c_led_init(I2cLed_t *i2c, const char *name, uint8_t addr, uint8_t output_reg)
{
    i2c->base.name = name;
    i2c->base.is_on = false;
    i2c->base.ops = &i2c_led_ops;
    i2c->addr = addr;
    i2c->output_reg = output_reg;
}

void led_on(Led_t *led)
{
    if (led == NULL || led->ops == NULL || led->ops->on == NULL) {
        return;
    }

    led->ops->on(led);
}

void led_container_of_demo(void)
{
    PwmLed_t breath_led;
    I2cLed_t status_led;
    Led_t *led;

    pwm_led_init(&breath_led, "breath_led", 80);
    i2c_led_init(&status_led, "status_led", 0x20, 0x02);

    printf("=== Step 1: Led_t base ptr -> PwmLed_t ===\n");
    printf("  offsetof(PwmLed_t, base)=%zu\n", offsetof(PwmLed_t, base));
    led = (Led_t *)&breath_led;
    led_on(led);

    printf("\n=== Step 2: Led_t base ptr -> I2cLed_t ===\n");
    printf("  offsetof(I2cLed_t, base)=%zu\n", offsetof(I2cLed_t, base));
    led = (Led_t *)&status_led;
    led_on(led);
}
