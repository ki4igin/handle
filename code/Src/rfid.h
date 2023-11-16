#ifndef __RFID_H
#define __RFID_H

#include "platform.h"
#include "st_errno.h"

#define RFID_CARD_UID_LEN_MAX 10

struct rfid_card_uid {
    uint32_t len;
    uint8_t val[RFID_CARD_UID_LEN_MAX];
};

extern struct rfid_card_uid rfid_card_uid;

uint32_t rfid_cycle(void);

#endif
