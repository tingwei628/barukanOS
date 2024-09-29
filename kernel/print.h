#ifndef PRINT_H
#define PRINT_H

#define SCREEN_MAX_COL      80
#define SCREEN_MAX_ROW      25
#define LINE_SIZE           160 // SCREEN_MAX_COL * 2
#define VGA_START           (uint8_t*)0xb8000

#define COLOR_WHITE         0xf

#include "stdint.h"

typedef struct ScreenBuffer {
    int32_t column;
    int32_t row;
} ScreenBuffer;


uint32_t printk(const uint8_t *format, ...);

#endif