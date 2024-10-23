#ifndef LIB_USER_H
#define LIB_USER_H

#include "stdint.h"
#include "stddef.h"

int32_t printf(const char *format, ...);
void sleepu(uint64_t ticks);
void exitu(void);
void waitu(void);
uint8_t keyboard_readu(void);
int32_t get_total_memoryu(void);

int32_t memcmp_u(void *src1, void *src2, size_t size);

int32_t read_cmd(char *buffer);
int32_t parse_cmd(char *buffer, int32_t buffer_size);
void execute_cmd(int32_t cmd);

#endif