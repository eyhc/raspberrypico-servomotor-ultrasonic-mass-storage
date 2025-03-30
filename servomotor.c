#include "servomotor.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

void servo_init()
{
    // 125 Mhz / 80 = 1562.5 kHz
    // 20 ms * 1562.5 kHz = 31250

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 80);
    pwm_config_set_wrap(&config, 31250);
    
    gpio_set_function(SERVOMOTOR_PULSE_PIN, GPIO_FUNC_PWM);
    pwm_init(pwm_gpio_to_slice_num(SERVOMOTOR_PULSE_PIN), &config, true);
    pwm_set_gpio_level(SERVOMOTOR_PULSE_PIN, 0);
}

void servo_set_angle(uint8_t angle)
{
    if (angle > 180) 
        angle = 180;

    //   0° -> 1ms : 1 ms * 1562.5 kHz = 1563
    // 180° -> 2ms : 2 ms * 1562.5 kHz = 3125

    // in practice
    // [0, 180] -> [1400, 3300]
    uint16_t level = (1900 * angle) / 180  + 1400;

    pwm_set_gpio_level(SERVOMOTOR_PULSE_PIN, level);
}
