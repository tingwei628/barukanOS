#ifndef MEMORY_H
#define MEMORY_H

#include "stdint.h"
#include "stddef.h"

#define MEMORY_ENTRY_NUMBER_ADDR        0x9000  // from get_memory_info
#define MEMORY_MAP_ADDR                 0x9008  // from get_memory_info
#define E820_TYPE_RAM                   1
#define KERNEL_VIRTUAL_MEMORY_START     0xffff800000000000

#define PAGE_SIZE           (2 * 1024 * 1024) // 2MB
#define PA_UP(v)            ((((uint64_t) v + PAGE_SIZE - 1) >> 21) << 21) // the upper align address after v
#define PA_DOWN(v)          (((uint64_t) v >> 21) << 21) // the lower align address before v 
#define P2V(p)              ((uint64_t)(p) + KERNEL_VIRTUAL_MEMORY_START) // physical address to virtual address relative to kernel base adderess
#define V2P(v)              ((uint64_t)(v) - KERNEL_VIRTUAL_MEMORY_START) // virtual address to physical address relative kernel base adderesss


typedef struct __attribute__((packed)) {
    uint64_t address;
    uint64_t length;
    uint32_t type;
} E820;


typedef struct FreeMemRegion {
    uint64_t address;
    uint64_t length;
} FreeMemRegion;

typedef struct Page {
    struct Page *next;
} Page;


void* kmemset(void *buffer, char value, size_t size);
void* kmemmove(void *dst, void *src, size_t size);
void* kmemcpy(void *dst, void *src, size_t size);
int32_t kmemcmp(void *src1, void *src2, size_t size);

void* kmalloc(void);
void kfree(uint64_t v);

uint64_t init_memory(void);
uint64_t get_total_memory(void);

#endif