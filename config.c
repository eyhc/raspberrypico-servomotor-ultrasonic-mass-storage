#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/flash.h"
#include "pico/flash.h"
#include "inih/ini.h"

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



/***********************************
 *        INI FILE OUTPUT
 */

static char ini_string_fmt[] = "\
# Fichier de configuration du projet with a servomotor and a ultrasonic sensor\n\
\n[servomotor]\n\
# Les angles sont compris entre 0 et 180 ° !\n\
fst_angle = %u\n\
snd_angle = %u\n\
# Delai entre le passage d'un angle à l'autre en (ms) entre 100 et 5000\n\
switch_angle_delay_ms = %u\n\
\n[ultrasonic]\n\
# threshold_distance entre 2 et 100 !\n\
threshold_distance = %f\n\
";

void write_ini_config(const config *conf, FIL *fp) {
    char temp[2 * sizeof(ini_string_fmt)];
    sprintf(temp, ini_string_fmt, 
        conf->fst_angle,
        conf->snd_angle,
        conf->switch_angle_delay_ms,
        conf->threshold_distance
    );

    size_t len = strlen(temp);

    unsigned int bw;
    f_write(fp, temp, len, &bw);
}


/***********************************
 *        INI FILE INPUT
 */
char *ini_reader_fil(char *str, int num, void* stream) {
    FIL *fp = (FIL*) stream;
    int nc = 0;
    char *p = str;
    char c;
    UINT rc;

	/* Byte-by-byte read */
	num -= 1;	/* Make a room for the terminator */
	while (nc < num) {
		f_read(fp, &c, 1, &rc);	/* Get a byte */
		if (rc != 1) break;		/* EOF? */
		if (c == '\r') continue;
		
        *p++ = c;
        nc++;

        if (c == '\n') break;
	}

	*p = 0;		/* Terminate the string */
	return nc ? str : NULL;
}

int handler(void *user, const char *section, const char *name, const char *value) {
    config *conf = (config*) user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("servomotor", "fst_angle")) {
        conf->fst_angle = atoi(value);
    }
    else if (MATCH("servomotor", "snd_angle")) {
        conf->snd_angle = atoi(value);
    }
    else if (MATCH("servomotor", "switch_angle_delay_ms")) {
        conf->switch_angle_delay_ms = atoi(value);
    }
    else if (MATCH("ultrasonic", "threshold_distance")) {
        conf->threshold_distance = atof(value);
    }
    return 1;
}

config read_ini_config(FIL *fp) {
    config conf;
    conf.magic_word = MAGIC_WORD;

    ini_parse_stream((ini_reader) ini_reader_fil, fp, (ini_handler)handler, &conf);

    return conf;
}