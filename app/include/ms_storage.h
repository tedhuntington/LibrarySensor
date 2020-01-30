/**
 * Handles persistent local data storage access
 */

#ifndef _MS_STORAGE_H_
#define _MS_STORAGE_H_

#include <stdint.h>
#include "nvs.h" // nvs_handle

#define NVS_BASE_NAME "STORAGE"

typedef nvs_handle nvs_handle_t; // future-proofing. when Mongoose upgrades to ESP-IDF 4, remove this line

void init_storage(void);

bool save_u32(const char *name, uint32_t value);
bool load_u32(const char *name, uint32_t *value);

#endif
