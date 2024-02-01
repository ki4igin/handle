#ifndef __ERR_H
#define __ERR_H

#include "stdint.h"

union err {
    struct {
        uint16_t double_save_card :1;
        uint16_t limit_save_card  :1;
    };

    uint16_t raw;
};

extern union err err;

#endif
