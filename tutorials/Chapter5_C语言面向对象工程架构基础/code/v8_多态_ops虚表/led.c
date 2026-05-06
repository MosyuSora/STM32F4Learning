#include "led.h"
#include <stdio.h>
static void normal_on(Led_t *led){led->is_on=true;printf("  [Normal] Pin%d -> ON\n",led->pin);}
static void normal_off(Led_t *led){led->is_on=false;printf("  [Normal] Pin%d -> OFF\n",led->pin);}
static void normal_toggle(Led_t *led){if(led->is_on)normal_off(led);else normal_on(led);}
static const LedOps_t normal_ops={.on=normal_on,.off=normal_off,.toggle=normal_toggle};
static void pwm_on(Led_t *led){PwmLed_t *real=(PwmLed_t*)led;real->base.is_on=true;printf("  [PwmLED] Pin%d -> ON, duty=%d%%\n",real->base.pin,real->duty);}
static void pwm_off(Led_t *led){PwmLed_t *real=(PwmLed_t*)led;real->base.is_on=false;printf("  [PwmLED] Pin%d -> OFF\n",real->base.pin);}
static void pwm_toggle(Led_t *led){if(led->is_on)pwm_off(led);else pwm_on(led);}
static const LedOps_t pwm_ops={.on=pwm_on,.off=pwm_off,.toggle=pwm_toggle};
int led_init(Led_t *led, uint8_t pin, const LedOps_t *ops){led->pin=pin;led->is_on=false;led->ops=ops;return 0;}
int led_on(Led_t *led){led->ops->on(led);return 0;}
int led_off(Led_t *led){led->ops->off(led);return 0;}
int led_toggle(Led_t *led){led->ops->toggle(led);return 0;}
int pwm_led_init(PwmLed_t *pwm, uint8_t pin, uint8_t duty){led_init(&pwm->base,pin,&pwm_ops);pwm->duty=duty;return 0;}
const LedOps_t *led_get_normal_ops(void){return &normal_ops;}