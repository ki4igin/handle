#include "access.h"

#define ACCESS_COUNT_MAX 64

static uint32_t access_cur_uid = 0;
static uint32_t last_read_uid = 0;
fifo_declare(access, struct access, ACCESS_COUNT_MAX);

void access_init(void)
{
    access_fifo->head = 1;
    access_fifo->tail = 1;
}

struct access access_create(union rfid_card_uid u, uint32_t time_ms)
{
    return (struct access){
        .acc_uid = ++access_cur_uid,
        .card_uid = u,
        .time_ms = time_ms,
    };
}

uint32_t access_set_last_read_uid(uint32_t uid)
{
    uint32_t uid_is_valid = uid >= last_read_uid && uid <= access_cur_uid;
    if (uid_is_valid) {
        last_read_uid = uid;
        access_fifo_mov_tail(access_fifo, uid + 1);
    }
    return uid_is_valid;
}

uint32_t access_get_last_read_uid(void)
{
    return last_read_uid;
}

uint32_t access_get_cur_uid(void)
{
    return access_cur_uid;
}

uint32_t access_get_non_read_count(void)
{
    return access_cur_uid - last_read_uid;
}
