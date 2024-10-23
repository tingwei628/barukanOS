#include "string.h"

static const char digits_decimal_map[10] = "0123456789";
static const char digits_hex_map[16] = "0123456789ABCDEF";

int32_t digits_to_string(uint8_t *digits_buffer, uint64_t digits)
{
    int32_t size = 0;
    
    do {
        digits_buffer[size++] = digits_decimal_map[digits % 10];
        digits /= 10;
    } while (digits != 0);

    return size;
}


int32_t digits_hex_to_string(uint8_t *digits_buffer, uint64_t digits)
{
    
    int32_t size = 0;

    do {
        digits_buffer[size++] = digits_hex_map[digits % 16];
        digits /= 16;
    } while (digits != 0);

    return size;
}
