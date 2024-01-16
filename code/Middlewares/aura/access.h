#ifndef __ACCESS_H
#define __ACCESS_H

#include "stdint.h"
#include "rfid.h"

#define ACCESS_COUNT_MAX 2

struct access {
    union rfid_card_uid uid;
    uint32_t time_ms;
    uint32_t is_valid;
};

static struct {
    uint32_t count;
    struct access data[ACCESS_COUNT_MAX];
} access_buf = {0};

inline static void access_add(struct access *acc)
{
    access_buf.data[access_buf.count] = *acc;
    access_buf.count = (access_buf.count + 1) & (ACCESS_COUNT_MAX - 1);
}

inline static struct access *access_get_last(void)
{
    return &access_buf.data[access_buf.count - 1];
}

inline static void access_clear(void)
{
    access_buf.count = 0;
}

inline static uint32_t access_get_count(void)
{
    return access_buf.count;
}

inline static struct access *access_get(uint32_t idx)
{
    return &access_buf.data[idx];
}

#endif
