#ifndef LIB_USER_H
#define LIB_USER_H

#include "stdint.h"

#define SYSCALL_ASM "int $0x80\n"

int32_t printf(const uint8_t *format, ...);
void sleepu(uint64_t ticks);
void exitu(void);
void waitu(void);
int32_t writeu(uint8_t *buffer, int32_t buffer_size);

#endif