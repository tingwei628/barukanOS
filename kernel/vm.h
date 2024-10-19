#ifndef VM_H
#define VM_H

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

#define PTE_P       1       // page table entry is present = bit-0 = 1
#define PTE_W       2       // page table entry Read/write = bit-1 = 1 
#define PTE_U       4       // page can be access in user mode = bit-2 = 1
#define PTE_ENTRY   0x80    // page size 2MB = bit-7 = 1 in PD entry


// p is PML4 entry 
// get physical address of PDP table in p
// OR 
// p is PDP entry 
// get physical address of PD table in p
#define PDE_ADDR(p) (((uint64_t)p >> 12) << 12)

// get physical address of 2MB page frame in PD entry
#define PTE_ADDR(p) (((uint64_t)p >> 21) << 21)

typedef uint64_t    *PD;     // pointer to PD entry in virtual memory
typedef uint64_t    **PDPTR; // pointer to PDP entry in virtual memory

void init_kvm(uint64_t memory_end);
bool map_pages(uint64_t map, uint64_t v, uint64_t e, uint64_t pa, uint32_t attribute);
void switch_vm(uint64_t map);
void free_vm(uint64_t map);
void free_page(uint64_t map, uint64_t v, uint64_t e);
bool setup_uvm(uint64_t map, uint64_t start, int size);
uint64_t setup_kvm();

#endif