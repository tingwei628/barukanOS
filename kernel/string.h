#ifndef STRING_H
#define STRING_H

#include "stdint.h"

int32_t digits_to_string(uint8_t *digits_buffer, uint64_t digits);
int32_t digits_hex_to_string(uint8_t *digits_buffer, uint64_t digits);


#endif