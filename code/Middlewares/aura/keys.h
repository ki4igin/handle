#ifndef __KEYS_H
#define __KEYS_H

#include <stdint.h>
#include <assert.h>
#include "rfid.h"

struct keys_range {
    uint8_t *p;
    uint32_t size;
};

void keys_init(void);
uint32_t key_is_valid(union rfid_card_uid *uid);
uint32_t key_save(union rfid_card_uid *uid);
uint32_t keys_get_count(void);
uint32_t keys_clear(void);
struct keys_range keys_get_cards(uint32_t offset, uint32_t count);

#endif
