#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/time.h"
#include "tusb.h"
#include "tusb_config.h"
#include "config.h"
#include "servomotor.h"
#include "ultrasonic.h"
#include "tusb_msc.h"


// We'll use a region 512k from the start of flash.
#define CONFIG_OFFSET (128 * FLASH_SECTOR_SIZE)


/************************************
 *           INIT TASK
 ************************************/
static void init_task() {
    // init harware
    servo_init();
    ultrasonic_init();
    
    // init flash for config
    init_config_flash(CONFIG_OFFSET);

    // init tiny usb device
    tud_init(BOARD_TUD_RHPORT);
    
    // init stdio for CDC communication
    stdio_init_all();
}


/************************************
 *   WRITE INI CONFIG FILE TASK
 ************************************/
static void write_config_file_task(const config *conf) {
    FATFS filesystem;
    f_mount(&filesystem, "/", 1);

    FIL fp;
    f_open(&fp, "config.ini", FA_CREATE_ALWAYS | FA_WRITE);
    write_ini_config(conf, &fp);
    f_close(&fp);

    f_unmount("/");
}


/************************************
 *   READ INI CONFIG FILE TASK
 ************************************/
void read_ini_file_task(config *conf) {
    FATFS filesystem;
    f_mount(&filesystem, "/", 1);

    FIL fp;
    f_open(&fp, "config.ini", FA_READ);
    config c = read_ini_config(&fp);
    f_close(&fp);

    printf("Config from ini file : \n");
    print_config(&c);

    if (!equals_config(&c, conf)) {
        if (is_correct_config(&c)) {
            write_config_flash(&c, CONFIG_OFFSET);
            (*conf) = c;
        }
        else {
            printf("invalid config\n");
            f_open(&fp, "config.ini", FA_CREATE_ALWAYS | FA_WRITE);
            write_ini_config(conf, &fp);
            f_close(&fp);
        }
    }

    f_unmount("/");
}


/************************************
 *           MAIN FSM
 ************************************/

typedef enum {
    INIT,
    MEASURE,
    SWITCH,
    WAIT
} state_t;

void main_fsm_task(const config *conf) {
    static uint8_t curr_angle;
    static uint8_t prev_angle;
    static absolute_time_t start_time;
    static state_t state = INIT;

    // the state machine !
    switch (state) {
    case INIT:
        curr_angle = conf->fst_angle;
        servo_set_angle(curr_angle);
        state = MEASURE;
        printf("go to MEASEARE\n----------\n");
        break;
    
    case MEASURE:
        // get value from ultrasonic sensor
        float distance = get_distance_ultrasonic(100000); // 100 ms max
        printf("distance: %f\n", distance);
        if (distance < 0) return;

        // if it's the correct angle ...
        if (
            (distance >= conf->threshold_distance && curr_angle == conf->fst_angle) ||
            (distance < conf->threshold_distance  && curr_angle == conf->snd_angle)
        ) { // ...nothing to do
            printf("go to WAIT\n----------\n");
            state = WAIT;
        }
        else { // otherwise, switch servo position
            prev_angle = curr_angle;
            printf("go to SWITCH\n----------\n");
            state = SWITCH;
        }

        // get timestamp
        start_time = get_absolute_time();
        break;
    
    case WAIT:
        int64_t waiting_time = absolute_time_diff_us(start_time, get_absolute_time());
        if (waiting_time >= conf->time_between_measures_ms * 1000) {
            printf("go to MEASEARE\n----------\n");
            state = MEASURE;
        }
        break;
    
    case SWITCH:
        uint8_t expected = (prev_angle == conf->fst_angle) ? conf->snd_angle : conf->fst_angle;
        if (curr_angle == expected) {
            printf("go to WAIT\n----------\n");
            state = WAIT;
        }
        else {
            uint64_t delta = abs(expected - prev_angle);
            delta *= (get_absolute_time() - start_time);
            delta /= (conf->switch_angle_delay_ms * 1000);
            if (prev_angle > expected) curr_angle = MAX(prev_angle - delta, expected);
            if (prev_angle < expected) curr_angle = MIN(prev_angle + delta, expected);
            servo_set_angle(curr_angle);
        }
        break;
    }
}

// ------------------------------------------------------------------------------------

static bool usb_file_written = false;

void usb_event_callback(event evt) {
    if (evt.type == READ_CALL) {
        printf("READ sector %u, offset = %u, size = %u\n", 
               evt.lba, evt.offset, evt.bufsize);
    }
    else if (evt.type == WRITE_CALL) {
        printf("WRITE sector %u, offset = %u, size = %u\n", 
            evt.lba, evt.offset, evt.bufsize);
        
        if (evt.lba != 0)
            usb_file_written = true;
    }
}

int main() {
    // INIT HARWARE
    init_task();

    // init config params
    static config current_config;
    current_config = read_config_flash(CONFIG_OFFSET);
    printf("config from flash\n");
    print_config(&current_config);

    // write config on msc_disk
    write_config_file_task(&current_config);

    // set ram_disk callback
    usb_set_callback(&usb_event_callback);

    /* MAIN PROGRAM */
    while (1) {
        // read_config_task
        if(usb_file_written) {
            read_ini_file_task(&current_config);
            usb_file_written = false;
        }

        // tiny usb device default task
        tud_task();

        // main task (servo & ultrasonic sensor)
        main_fsm_task(&current_config);

        sleep_ms(10);
    }
}
