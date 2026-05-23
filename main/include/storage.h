#ifndef _STORAGE_H
#define _STORAGE_H

#include <common.h>
#include "nvs_flash.h"
#include "nvs.h"

#define STORAGE_NAME "storage"

bool flash_storage_init(void);
bool flash_write(const char *data_name, const void *write_buffer, size_t length);
bool flash_read(const char *data_name, void *read_buffer, size_t length);

#endif