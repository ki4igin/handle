#ifndef __ACCESS_H
#define __ACCESS_H

#include "stdint.h"
#include "rfid.h"
#include "circ.h"

struct access {
    union rfid_card_uid uid;
    uint32_t time_ms;
};

extern struct circ *const access_circ;

circ_func_define(access, struct access)

#endif
