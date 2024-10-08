#ifndef DEBUG_H
#define DEBUG_H

#include "stdint.h"

#define ASSERT(e) do {                      \
        if (!(e))                           \
            error_check(__FILE__,__func__,__LINE__); \
} while (0) 

void error_check(uint8_t *file, uint8_t const *func, uint64_t line);

#endif