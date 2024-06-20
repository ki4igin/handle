#include "access.h"

#define ACCESS_COUNT_MAX 64

uint32_t access_cur_uid = 0;
static uint32_t last_read_uid = 0;
fifo_declare(access, struct access, ACCESS_COUNT_MAX);

void access_set_last_read_uid(uint32_t uid)
{
    last_read_uid = uid;
    access_fifo_mov_head(access_fifo, uid + 1);
}

uint32_t access_get_non_read_count(void)
{
    return access_cur_uid - last_read_uid;
}