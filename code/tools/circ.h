#ifndef __CIRC_H
#define __CIRC_H

#include <stdint.h>
#include <stddef.h>
#include "memcpy.h"

struct circ {
    const size_t mask;
    size_t head;
    size_t data[];
};

// clang-format off
#define circ_declare(_name, _type, _count)               \
    struct circ *const                                   \
        _name##_circ = (struct circ *const)&(struct {    \
            const size_t mask;                           \
            size_t head;                                 \
            _type data[_count];                          \
        }){.mask = (_count) - 1}
// clang-format on

#define circ_inc(_c, _type)                   \
    do {                                      \
        _c->head = (_c->head + 1) & _c->mask; \
    } while (0)

#define circ_func_define(_prefix, _type)                                     \
    inline static void _prefix##_circ_add(struct circ *const c, _type *data) \
    {                                                                        \
        size_t len = sizeof(_type) / sizeof(size_t);                         \
        memcpy128(sizeof(_type), data, &c->data[c->head * len]);             \
        circ_inc(c, _type);                                                  \
    }                                                                        \
                                                                             \
    inline static _type *                                                    \
        _prefix##_circ_get_from_end(struct circ *const c, size_t idx)        \
    {                                                                        \
        size_t len = sizeof(_type) / sizeof(size_t);                         \
        size_t idx_abs = (c->head - idx - 1) & (c->mask);                    \
        return (_type *)&c->data[idx_abs * len];                             \
    }                                                                        \
                                                                             \
    inline static _type *_prefix##_circ_get_last(struct circ *const c)       \
    {                                                                        \
        return _prefix##_circ_get_from_end(c, 0);                            \
    }

#endif
