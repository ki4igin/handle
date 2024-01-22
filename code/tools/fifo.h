#ifndef __FIFO_H
#define __FIFO_H

#include <stdint.h>
#include <stddef.h>
#include "memcpy.h"

struct fifo {
    const size_t mask;
    size_t head;
    size_t tail;
    const size_t *data;
};

#define fifo_declare(_name, _type, _count)                                     \
    static size_t                                                              \
        _name##_fifo_storage[sizeof(_type) * (_count) / sizeof(size_t)] = {0}; \
    struct fifo                                                                \
        _name##_fifo = {.mask = (_count)-1, .data = _name##_fifo_storage}

inline static size_t fifo_is_empty(struct fifo *f)
{
    return f->head == f->tail;
}

inline static size_t fifo_not_empty(struct fifo *f)
{
    return f->head != f->tail;
}

inline static size_t fifo_is_full(struct fifo *f)
{
    return ((f->head + 1) & (f->mask)) == f->tail;
}

#define fifo_inc(_f, _type, _ht)            \
    do {                                    \
        _f->_ht = (_f->_ht + 1) & _f->mask; \
    } while (0)

#define fifo_func_define(_prefix, _type)                               \
    inline static void                                                 \
        _prefix##fifo_push_unchecked(struct fifo *f, _type *data)      \
    {                                                                  \
        const size_t len = sizeof(_type) / sizeof(size_t);             \
        memcpy128(sizeof(_type), data, &f->data[f->head * len]);       \
        fifo_inc(f, _type, head);                                      \
    }                                                                  \
                                                                       \
    inline static void _prefix##fifo_push(struct fifo *f, _type *data) \
    {                                                                  \
        if (fifo_is_full(f)) {                                         \
            fifo_inc(f, _type, tail);                                  \
        }                                                              \
        _prefix##fifo_push_unchecked(f, data);                         \
    }                                                                  \
                                                                       \
    inline static _type *_prefix##fifo_pop(struct fifo *f)             \
    {                                                                  \
        const size_t len = sizeof(_type) / sizeof(size_t);             \
        _type *p = (_type *)&f->data[f->tail * len];                   \
        fifo_inc(f, _type, tail);                                      \
        return p;                                                      \
    }

#endif
