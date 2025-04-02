#include "ultrasonic.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/stdlib.h"

void ultrasonic_init() {
    // Serial IO init as IN dir
    gpio_init(ULTRASONIC_ECHO_PIN);

    gpio_init(ULTRASONIC_TRIG_PIN);
    gpio_set_dir(ULTRASONIC_TRIG_PIN, GPIO_OUT);
}

float get_distance_ultrasonic(uint32_t timeout_us)
{
    // minimum between to trig
    if (timeout_us < 60000) timeout_us = 600000;

    // set trig to 1 for 10 µs
    gpio_put(ULTRASONIC_TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(ULTRASONIC_TRIG_PIN, 0);

    absolute_time_t time = get_absolute_time();
    int64_t duration;

    // waiting for rising edge
    while(gpio_get(ULTRASONIC_ECHO_PIN) == 0 && 
            absolute_time_diff_us(time, get_absolute_time()) < timeout_us);

    if (absolute_time_diff_us(time, get_absolute_time()) >= timeout_us)
        return -1.f;

    // waiting for falling edge
    time = get_absolute_time();
    while(gpio_get(ULTRASONIC_ECHO_PIN) == 1)
        duration = absolute_time_diff_us(time, get_absolute_time());

    // distance (cm) = SOUND_SPEED (m/s) * duration(µs)/2 * 10^(-3) (s/µs) * 10^-1(m/cm)
    return 340.0f * duration / 20000;
}
