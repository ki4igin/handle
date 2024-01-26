#ifndef __DEBUG_H
#define __DEBUG_H

#include "stdint.h"

#define debug_error_handler() _debug_error_handler(__FILE__, __LINE__)

void debug_send(void *p, uint32_t size);
void debug_printf(const char *fmt, ...);
void _debug_error_handler(const char *file, uint32_t line);

#endif
