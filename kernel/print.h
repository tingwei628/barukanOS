#ifndef PRINT_H
#define PRINT_H

#include "stdint.h"

#define SCREEN_MAX_COL      80
#define SCREEN_MAX_ROW      25
#define LINE_SIZE           160 // SCREEN_MAX_COL * 2
#define VGA_START           0xb8000 // vga physical memory start

#define COLOR_WHITE         0xf

typedef struct ScreenBuffer {
    int32_t column;
    int32_t row;
} ScreenBuffer;


uint32_t printk(const char *format, ...);
void write_screen(const char *buffer, int32_t size, char color);

#endif