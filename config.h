#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "ff.h"

typedef struct {
    uint16_t magic_word;
    uint8_t fst_angle;
    uint8_t snd_angle;
    uint16_t switch_angle_delay_ms;
    float threshold_distance;
} config;

extern const config default_config;

void print_config(const config *conf);
bool is_valid_magic_word(const config* conf);
bool is_correct_config(const config* conf);
bool equals_config(const config *c1, const config *c2);

// --- FLASH ---

// if config in flash is invalid, default config will be written
void init_config_flash(uint32_t offset);
config read_config_flash(uint32_t offset);
// caution: 4096 (FLASH_SECTOR_SIZE) bytes from offset will be erased
void write_config_flash(const config *conf, uint32_t offset);


// --- FILE API ---

void write_ini_config(const config *conf, FIL *fp);
config read_ini_config(FIL *fp);

#endif
