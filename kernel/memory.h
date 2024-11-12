#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"
#include "stddef.h"

void* kmalloc(void);
void kfree(uint64_t v);

uint64_t init_memory(void);
uint64_t get_total_memory(void);

#endif