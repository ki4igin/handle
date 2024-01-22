#ifndef __MEMCPY_H
#define __MEMCPY_H

#include <stdint.h>
#include <stddef.h>

#if !(defined(uint128_t))
typedef struct {
    uint32_t raw[4];
} uint128_t;
#endif

#if !(defined(uint256_t))
typedef struct {
    uint32_t raw[8];
} uint256_t;
#endif

#define memcpy_core(_nbits, _nbits_pre, _size, _src, _dst) \
    do {                                                   \
        const size_t len = (_size) / ((_nbits) / 8);       \
        uint##_nbits##_t *s = (uint##_nbits##_t *)(_src);  \
        uint##_nbits##_t *d = (uint##_nbits##_t *)(_dst);  \
        for (size_t i = 0; i < len; i++) {                 \
            *d++ = *s++;                                   \
        }                                                  \
        memcpy##_nbits_pre(size % ((_nbits) / 8), s, d);   \
    } while (0)

#define memcpy_func_def(_nbits, _nbits_pre)                                          \
    inline static void memcpy##_nbits(const size_t size, void *src, const void *dst) \
    {                                                                                \
        memcpy_core(_nbits, _nbits_pre, size, src, dst);                             \
    }

inline static void memcpy4(const size_t size, void *src, void *dst)
{
    (void)size;
    (void)src;
    (void)dst;
}

// clang-format off
memcpy_func_def(8, 4)
memcpy_func_def(16, 8)
memcpy_func_def(32, 16)
memcpy_func_def(64, 32)
memcpy_func_def(128, 64)
memcpy_func_def(256, 128)
// clang-format on

#undef memcpy_core
#undef memcpy_func_def

#endif
