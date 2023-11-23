#ifndef __BUF_H
#define __BUF_H

#include <stdint.h>
#include "tools.h"

#define BUF_LIST_COUNT 3

struct buf {
    uint32_t size;
    void *p;
};

struct buf_list {
    uint32_t count;
    uint32_t idx;
    struct buf bufs[BUF_LIST_COUNT];
};

inline static struct buf *buf_list_get_next(struct buf_list *list)
{
    struct buf *b = &list->bufs[list->idx];
    list->idx++;
    list->count--;
    return b;
}

inline static uint32_t buf_list_is_empty(struct buf_list *list)
{
    return list->count == 0;
}

inline static uint32_t buf_list_is_nonempty(struct buf_list *list)
{
    return list->count != 0;
}

#endif