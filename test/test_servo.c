#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "../servomotor.h"


int main(int argc, char const *argv[])
{
    uint8_t increase = 1;
    uint8_t angle = 0;

    servo_init();

    // Init stdio
    stdio_init_all();
    printf("Hello from servo tester !\n");

    while (1)
    {
        if (increase && angle < 180)
            angle++;
        else if (!increase && angle > 0)
            angle--;
        else
            increase = !increase;

        servo_set_angle(angle);

        if (angle % 10 == 0)
            printf("Set angle : %u\n", angle);

        sleep_ms(50);
    }
    
    return 0;
}
