#ifndef __ACCESS_H
#define __ACCESS_H

#include "stdint.h"
#include "rfid.h"
#include "circ.h"

struct access {
    uint32_t acc_uid;
    union rfid_card_uid uid;
    uint32_t time_ms;
};

struct access_pack{
    struct access acc;
    uint16_t is_valid;
};

extern uint32_t access_cur_uid;
extern uint32_t access_last_read_uid;
extern struct circ *const access_circ;

circ_func_define(access, struct access)

static inline uint32_t access_get_non_read_count(void)
{
    return access_cur_uid - access_last_read_uid;
}

#endif
