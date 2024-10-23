#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"

// #define E0_SIGN     (1 << 0)
// #define SHIFT       (1 << 1)
// #define CAPS_LOCK   (1 << 2)

typedef enum {
    E0_SIGN     = (1 << 0),     // 0x01
    SHIFT       = (1 << 1),     // 0x02
    CAPS_LOCK   = (1 << 2)      // 0x04
} KeyFlag;

// PS/2
typedef struct KeyboardBuffer {
    char buffer[500];
    int32_t front;
    int32_t end;
    int32_t size;
} KeyboardBuffer;

char read_key_buffer(void);
void keyboard_handler(void);

#endif