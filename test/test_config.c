#include "config.h"
#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/flash.h"

// We'll use a region 64k from the start of flash.
#define CONFIG_OFFSET (16 * FLASH_SECTOR_SIZE)

int main() {
    // Init stdio
    stdio_init_all();

    // wait for usb
    while (!stdio_usb_connected())
        sleep_ms(10);

    printf("Hello from flash config tester !\n");

    config c = read_config_flash(CONFIG_OFFSET);
    printf("config in memory : \n");
    print_config(&c);

    printf("init config\n");
    init_config_flash(CONFIG_OFFSET);

    c = read_config_flash(CONFIG_OFFSET);
    printf("new config in memory : \n");
    print_config(&c);

    c.snd_angle = 90;
    printf("write second angle = 90 in flash\n");
    write_config_flash(&c, CONFIG_OFFSET);

    printf("new config in memory\n");
    config c2 = read_config_flash(CONFIG_OFFSET);
    print_config(&c2);

    while (1)
        sleep_ms(1000);

    return 0;
}
