#ifndef __UID_HASH_H
#define __UID_HASH_H

#include "stm32f0xx.h"

#define UID_ADDR 0x1FFFF7AC
#define UID      (*(uint32_t *)UID_ADDR)

uint32_t uid_hash(void);

#endif
