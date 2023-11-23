#ifndef __CRC16_H__
#define __CRC16_H__

#include "stm32f0xx.h"
#include "buf.h"

typedef uint16_t crc16_t;

crc16_t crc16_calc(const void *buf, uint32_t size);
crc16_t crc16_calc_continue(crc16_t crc, const void *buf, uint32_t size);
void crc16_add2pack(void *buf, uint32_t size);
void crc16_add2list(struct buf_list *list);

inline static uint32_t crc16_is_valid(const void *buf, uint32_t size)
{
    return crc16_calc(buf, size) == 0;
}

#endif
