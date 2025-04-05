#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <stdint.h>

#define ULTRASONIC_ECHO_PIN 14
#define ULTRASONIC_TRIG_PIN 15

void ultrasonic_init();

float get_distance_ultrasonic(uint32_t timeout_us);

#endif