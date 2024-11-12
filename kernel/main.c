#include "idt.h"
#include "print.h"
#include "debug.h"
#include "mem.h"
#include "memory.h"
#include "vm.h"
#include "process.h"
#include "syscall.h"
#include "../fs/fat16/impl.h"

// from linker.ld
extern char bss_start;
extern char bss_end;

void kernal_main(void)
{
    // clear .bss
    uint64_t size = (uint64_t)&bss_end - (uint64_t)&bss_start;
    kmemset(&bss_start, 0, size);

    init_idt();
    uint64_t virtual_free_memory_end = init_memory();
    init_kvm(virtual_free_memory_end);
    init_system_call();
    init_fs();
    init_process();
}