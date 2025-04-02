#include "config.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include <stdio.h>

#define MAGIC_WORD 0xEC25

const config default_config = {
    .magic_word = MAGIC_WORD,
    .fst_angle = 20,
    .snd_angle = 160,
    .switch_angle_delay_ms = 300,
    .threshold_distance = 20.f
};

void print_config(const config *conf) {
    printf(
        "config: mw=0x%X alpha1=%u alpha2=%u delay_ms=%u thres=%.2f\n",
        conf->magic_word,
        conf->fst_angle,
        conf->snd_angle,
        conf->switch_angle_delay_ms,
        conf->threshold_distance
    );
}

bool is_valid_magic_word(const config *conf) {
    return conf->magic_word == MAGIC_WORD;
}

bool is_correct_config(const config *conf) {
    return is_valid_magic_word(conf) && 
            conf->fst_angle < conf->snd_angle &&
            conf->snd_angle < 180 &&
            conf->snd_angle - conf->fst_angle >= 10 &&
            conf->switch_angle_delay_ms >= 100 &&
            conf->switch_angle_delay_ms < 5000 &&
            conf->threshold_distance > 2.f &&
            conf->threshold_distance < 100.f;
}

bool equals_config(const config *c1, const config *c2) {
    return c1->fst_angle == c2->fst_angle &&
            c1->snd_angle == c2->snd_angle &&
            c1->switch_angle_delay_ms == c2->switch_angle_delay_ms &&
            c1->threshold_distance == c2->threshold_distance;
}


/***********************************
 *      FLASH PROGRAMMING
 */

static void _call_flash_erase(void *param) {
    uint32_t flash_target_offset = (uint32_t)param;

    // CAUTION /!\
    // offset  MUST be aligned to a 4096-byte flash sector
    // size    MUST be a multiple of 4096 bytes (one sector)
    flash_range_erase(flash_target_offset, FLASH_SECTOR_SIZE);
}

struct _flash_program_conf_args {
    uint32_t flash_offset;
    const config *config;
};

static void _call_flash_program_config(void *param) {
    struct _flash_program_conf_args* args = (struct _flash_program_conf_args*)param;

    const uint32_t offset = args->flash_offset;

    uint8_t *conf = (uint8_t *)args->config;
    uint8_t data[FLASH_PAGE_SIZE];

    for (size_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        data[i] = (i < sizeof(config)) ? conf[i] : 0xff;
    }

    // CAUTION /!\
    // offset  MUST be aligned to a 256-byte flash page
    // size MUST be a multiple of 256 bytes (one page)
    flash_range_program(offset, data, FLASH_PAGE_SIZE);
}


void init_config_flash(uint32_t offset) {
    config *conf_in_memory = (config *)(XIP_BASE + offset);

    if (!is_correct_config(conf_in_memory)) {
        write_config_flash(&default_config, offset);
    }
}

config read_config_flash(uint32_t offset) {
    config *conf_in_memory = (config *)(XIP_BASE + offset);
    return *conf_in_memory;
}

void write_config_flash(const config *conf, uint32_t flash_offset) {
    int rc;

    // erase the first sector after the given offset
    rc = flash_safe_execute(_call_flash_erase, (void*)flash_offset, UINT32_MAX);
    hard_assert(rc == PICO_OK);

    // write config in the first page of the sector
    struct _flash_program_conf_args param = {flash_offset, conf};
    rc = flash_safe_execute(_call_flash_program_config, &param, UINT32_MAX);
    hard_assert(rc == PICO_OK);
}
