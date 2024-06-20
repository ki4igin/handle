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

extern uint32_t access_cur_uid;
extern struct fifo *const access_fifo;

inline static struct access access_create(union rfid_card_uid u, uint32_t time_ms)
{
    return (struct access){
        .acc_uid = ++access_cur_uid,
        .card_uid = u,
        .time_ms = time_ms,
    };
}

uint32_t access_get_non_read_count(void);
void access_set_last_read_uid(uint32_t uid);

fifo_func_define(access, struct access)

#endif
