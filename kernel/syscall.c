#include "stddef.h"
#include "syscall.h"
#include "process.h"
#include "print.h"
#include "debug.h"
#include "memory.h"
#include "../drivers/keyboard.h"

static SYSTEMCALL system_calls[10];

static int32_t sys_write(int64_t *argptr)
{    
    write_screen((char*)argptr[0], (int32_t)argptr[1], 0xe);
    //write_screen((char*)argptr[0], (int32_t)argptr[1], (char)argptr[2]);
    return (int32_t)argptr[1];
}

static int32_t sys_sleep(int64_t *argptr)
{
    uint64_t old_ticks; 
    uint64_t ticks;
    uint64_t sleep_ticks = argptr[0];

    ticks = get_ticks();
    old_ticks = ticks;

    while (ticks - old_ticks < sleep_ticks)
    {
       sleep(-1);
       ticks = get_ticks();
    }

    return 0;
}

static int32_t sys_exit(int64_t *argptr)
{
    exit();
    return 0;
}

static int32_t sys_wait(int64_t *argptr)
{
    wait();
    return 0;
}
static int32_t sys_keyboard_read(int64_t *argptr)
{
    return read_key_buffer();
}

static int32_t sys_get_total_memory(int64_t *argptr)
{
    return get_total_memory();
}

void init_system_call(void)
{
    system_calls[0] = sys_write;
    system_calls[1] = sys_sleep;
    system_calls[2] = sys_exit;
    system_calls[3] = sys_wait;
    system_calls[4] = sys_keyboard_read;
    system_calls[5] = sys_get_total_memory;

    //system_call_max_number = 5
}

void system_call(TrapFrame *tf)
{
    int64_t i = tf->rax;
    int64_t param_count = tf->rdi;
    int64_t *argptr = (int64_t*)tf->rsi;
    int32_t system_call_max_number = 5;

    if (param_count < 0 || i > system_call_max_number || i < 0)
    { 
        tf->rax = -1;
        return;
    }
    
    ASSERT(system_calls[i] != NULL);
    tf->rax = system_calls[i](argptr);
}