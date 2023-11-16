#ifndef __CRC16_H__
#define __CRC16_H__

#include "stm32f0xx.h"

uint16_t crc16(uint16_t crc, const void *buf, uint32_t size);

#endif
