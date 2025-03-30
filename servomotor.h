#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

#include <stdint.h>

#define SERVOMOTOR_PULSE_PIN 22

void servo_init();
void servo_set_angle(uint8_t angle);

#endif