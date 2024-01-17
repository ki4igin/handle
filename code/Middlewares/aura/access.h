#ifndef __ACCESS_H
#define __ACCESS_H

#include "stdint.h"
#include "rfid.h"

#define ACCESS_COUNT_MAX 128

struct access {
    union rfid_card_uid uid;
    uint32_t time_ms;
};

struct access_buf {
    uint32_t count;
    struct access data[ACCESS_COUNT_MAX];
};

extern struct access_buf access_buf;

inline static void access_add(struct access *acc)
{
    access_buf.data[access_buf.count] = *acc;
    access_buf.count = (access_buf.count + 1) & (ACCESS_COUNT_MAX - 1);
}

inline static struct access *access_get_from_end(uint32_t idx_end)
{
    uint32_t idx = (access_buf.count - idx_end - 1) & (ACCESS_COUNT_MAX - 1);
    return &access_buf.data[idx];
}

inline static struct access *access_get_last(void)
{
    return access_get_from_end(0);
}

#endif
