/*
 * telemetry_storage.h
 *
 *  Created on: Feb 2, 2013
 *      Author: andyc
 */

#ifndef TELEMETRY_STORAGE_H_
#define TELEMETRY_STORAGE_H_

#include "emc.h"

#define TELEM_STORAGE_IDLIST_SIZE 256
#define TELEM_SENSOR_COUNT 40
#define TELEM_STORAGE_BASE_INDEX (STATIC_BANK_0_START_ADDR + 0x5000)
#define TELEM_STORAGE_BASE_ADDR (STATIC_BANK_0_START_ADDR + 0x5004)
#define TELEM_MAX_ENTRIES 2000

struct telem_storage_entry_t
{
    char values[TELEM_SENSOR_COUNT];
    unsigned int timestamp;
};

extern struct telem_storage_entry_t *cur;

void telemetry_storage_init(void);

void telemetry_storage_reset(void);

void telemetry_storage_save_cur(void);

void telemetry_storage_load_prev_cur(void);

int telemetry_storage_write(struct telem_storage_entry_t *buf);

int telemetry_storage_read_cur(struct telem_storage_entry_t *buf);

int telemetry_storage_read_index(unsigned int i, struct telem_storage_entry_t *buf);

void telemetry_print_entry_content(struct telem_storage_entry_t *entry);

#endif /* TELEMETRY_STORAGE_H_ */
