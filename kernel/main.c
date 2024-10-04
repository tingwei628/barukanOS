#include "idt.h"
#include "print.h"
#include "debug.h"
#include "memory.h"
#include "vm.h"

void kernal_main(void)
{
    init_idt();
    uint64_t virtual_free_memory_end = init_memory();
    init_kvm(virtual_free_memory_end);
    //uint8_t *string = "Hello barukanOS";
    //printk("%s", string);
}