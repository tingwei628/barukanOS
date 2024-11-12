#include "stddef.h"
#include "syscall.h"
#include "process.h"
#include "print.h"
#include "debug.h"
#include "memory.h"
#include "../drivers/keyboard.h"
#include "../fs/fat16/impl.h"

static SYSTEMCALL system_calls[SYSTEMCALL_NUM];

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
    // sleep_ticks is the duration passed from user program (sleepu)
    uint64_t sleep_ticks = argptr[0];

    ticks = get_ticks();
    old_ticks = ticks;

    while (ticks - old_ticks < sleep_ticks)
    {
       // -1 = waiting for ticks
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
    wait(argptr[0]);
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

static int32_t sys_open_file(int64_t *argptr)
{
    ProcessControl *pc = get_pc();
    return open_file(pc->current_process, (char*)argptr[0]);
}

static int32_t sys_read_file(int64_t *argptr)
{
    ProcessControl *pc = get_pc();
    return read_file(pc->current_process, argptr[0], (void*)argptr[1], argptr[2]);
}

static int32_t sys_close_file(int64_t *argptr)
{
    ProcessControl *pc = get_pc();
    close_file(pc->current_process, argptr[0]);

    return 0;
}

static int32_t sys_get_file_size(int64_t *argptr)
{
    struct ProcessControl *pc = get_pc();  
    return get_file_size(pc->current_process, argptr[0]);
}

static int32_t sys_fork(int64_t *argptr)
{
    return fork();
}

static int32_t sys_exec(int64_t *argptr)
{
    ProcessControl *pc = get_pc();
    Process *process = pc->current_process; 
    return exec(process, (char*)argptr[0]);
}

static int32_t sys_read_root_directory(int64_t *argptr)
{
    return read_root_directory((char*)argptr[0]);
}

void init_system_call(void)
{
    system_calls[0] = sys_write;
    system_calls[1] = sys_sleep;
    system_calls[2] = sys_exit;
    system_calls[3] = sys_wait;
    system_calls[4] = sys_keyboard_read;
    system_calls[5] = sys_get_total_memory;
    system_calls[6] = sys_open_file;
    system_calls[7] = sys_read_file;  
    system_calls[8] = sys_get_file_size;
    system_calls[9] = sys_close_file;
    system_calls[10] = sys_fork; 
    system_calls[11] = sys_exec; 
    system_calls[12] = sys_read_root_directory; 

    //system_call_max_number = 12
}

void system_call(TrapFrame *tf)
{
    // index number of syscall
    int64_t i = tf->rax;
    int64_t param_count = tf->rdi;
    // arguments pass to function 
    int64_t *argptr = (int64_t*)tf->rsi;
    int32_t system_call_max_number = 12;

    if (param_count < 0 || i > system_call_max_number || system_call_max_number >= SYSTEMCALL_NUM || i < 0)
    { 
        tf->rax = -1;
        return;
    }
    
    ASSERT(system_calls[i] != NULL);
    tf->rax = system_calls[i](argptr);
}