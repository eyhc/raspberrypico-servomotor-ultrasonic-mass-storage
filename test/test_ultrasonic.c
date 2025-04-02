#include "../ultrasonic.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

int main() {
    float distance;

    ultrasonic_init();

    // Init stdio
    stdio_init_all();

    // wait for usb
    // while (!stdio_usb_connected())
    //     sleep_ms(10);


    printf("Hello from ultrasonic sensor tester !\n");

    while (1)
    {
        printf("start read distance...\n");

        distance = get_distance_ultrasonic(10000000U);

        printf("read distance : %f cm\n", distance);

        sleep_ms(1000);
    }

    return 0;
}
