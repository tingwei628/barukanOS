#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"
#include "stddef.h"

void* kmemset(void *buffer, uint8_t value, size_t size);
void* kmemmove(void *dst, void *src, size_t size);
void* kmemcpy(void *dst, void *src, size_t size);
uint8_t kmemcmp(void *src1, void *src2, size_t size);

#endif