#include "access.h"

#define ACCESS_COUNT_MAX 128

uint32_t access_cur_uid = 0;
uint32_t access_last_read_uid = 0;
circ_declare(access, struct access, ACCESS_COUNT_MAX);
