#include "access.h"

#define ACCESS_COUNT_MAX 128

circ_declare(access, struct access, ACCESS_COUNT_MAX);
