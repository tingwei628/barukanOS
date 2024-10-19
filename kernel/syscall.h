#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt.h" 

typedef int32_t (*SYSTEMCALL)(int64_t *argptr); // function pointer of int(*)(int64_t *)
void init_system_call(void);
void system_call(TrapFrame *tf);

#endif