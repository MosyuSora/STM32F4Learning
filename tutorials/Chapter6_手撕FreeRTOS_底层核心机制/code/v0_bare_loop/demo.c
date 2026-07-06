#include <stdio.h>

static int now_ms = 0;
static int next_led_ms = 0;
static int next_sensor_ms = 0;
static int log_burst_left = 0;

static void led_heartbeat(void) {
    if (now_ms >= next_led_ms) {
        printf("t=%03d LED toggle\n", now_ms);
        next_led_ms += 50;
    }
}

static void sensor_sample(void) {
    if (now_ms >= next_sensor_ms) {
        printf("t=%03d SENSOR sample\n", now_ms);
        next_sensor_ms += 20;
    }
}

static int log_flush(void) {
    if (now_ms == 40) {
        log_burst_left = 3;
    }
    if (log_burst_left > 0) {
        printf("t=%03d LOG flush chunk, main loop blocked 35ms\n", now_ms);
        log_burst_left--;
        return 35;
    }
    return 5;
}

int main(void) {
    while (now_ms <= 140) {
        led_heartbeat();
        sensor_sample();
        now_ms += log_flush();
    }
    puts("result: LED and SENSOR are delayed by slow LOG work");
    return 0;
}

