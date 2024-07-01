#ifndef __ACCESS_H
#define __ACCESS_H

#include "stdint.h"
#include "rfid.h"
#include "fifo.h"

struct access {
    uint32_t acc_uid;
    union rfid_card_uid card_uid;
    uint32_t time_ms;
};

struct access_pack {
    struct access acc;
    uint16_t is_valid;
};

extern struct fifo *const access_fifo;

void access_init(void);
struct access access_create(union rfid_card_uid u, uint32_t time_ms);
uint32_t access_get_non_read_count(void);
uint32_t access_set_last_read_uid(uint32_t uid);
uint32_t access_get_last_read_uid(void);
uint32_t access_get_cur_uid(void);

fifo_func_define(access, struct access)

#endif
