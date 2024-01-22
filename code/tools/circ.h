#ifndef __CIRC_H
#define __CIRC_H

#include <stdint.h>
#include <stddef.h>
#include "memcpy.h"
#include "access.h"

struct circ {
    const size_t mask;
    size_t head;
    const size_t *data;
};

#define circ_declare(_name, _type, _count)                                     \
    static size_t                                                              \
        _name##_circ_storage[sizeof(_type) * (_count) / sizeof(size_t)] = {0}; \
    struct circ                                                                \
        _name##_circ = {.mask = (_count)-1, .data = _name##_circ_storage}

#define circ_inc(_c, _type)                   \
    do {                                      \
        _c->head = (_c->head + 1) & _c->mask; \
    } while (0)

#define circ_func_define(_prefix, _type)                               \
    inline static void _prefix##_circ_add(struct circ *c, _type *data) \
    {                                                                  \
        const size_t len = sizeof(_type) / sizeof(size_t);             \
        memcpy128(sizeof(_type), data, &c->data[c->head * len]);       \
        circ_inc(c, _type);                                            \
    }                                                                  \
                                                                       \
    inline static _type *                                              \
        _prefix##_circ_get_from_end(struct circ *c, size_t idx)        \
    {                                                                  \
        const size_t len = sizeof(_type) / sizeof(size_t);             \
        size_t idx_abs = (c->head - idx - 1) & (c->mask);              \
        return (_type *)&c->data[idx_abs * len];                       \
    }                                                                  \
                                                                       \
    inline static _type *_prefix##_circ_get_last(struct circ *c)       \
    {                                                                  \
        return _prefix##_circ_get_from_end(c, 0);                      \
    }

#endif
