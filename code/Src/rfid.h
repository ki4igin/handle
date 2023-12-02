#ifndef __RFID_H
#define __RFID_H

#include "platform.h"
#include "st_errno.h"

#define RFID_CARD_UID_LEN_MAX 7

union rfid_card_uid {
    struct {
        uint8_t len;
        uint8_t val[RFID_CARD_UID_LEN_MAX];
    };

    uint32_t raw[2];
};

extern __ALIGNED(4) union rfid_card_uid rfid_card_uid;

inline static void rfid_clear_card(union rfid_card_uid *uid)
{
    *uid = (union rfid_card_uid){0};
}

uint32_t rfid_cycle(void);

#endif
