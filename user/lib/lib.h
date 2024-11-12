#ifndef LIB_USER_H
#define LIB_USER_H

#include "stdint.h"
#include "stddef.h"

// Root Directory Secion
typedef struct __attribute__((packed)) {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_ms;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t attr_index;
    uint16_t m_time;
    uint16_t m_date;
    uint16_t cluster_index; // starting cluster 
    uint32_t file_size;
} DirEntry;

typedef enum {
    READ_ONLY      = 0x01,
    HIDDEN         = 0x02,
    SYSTEM         = 0x04,
    VOLUME_LABEL   = 0x08,
    LONG_FILE_NAME = 0x0f,
    DIRECTORY      = 0x10,
    ARCHIEVE       = 0x20
} FileAttribute;

typedef enum {
    EMPTY     = 0,
    DELETED   = 0xe5
} EntryAllocationStatus;



int32_t printf(const char *format, ...);
void sleepu(uint64_t ticks);
void exitu(void);
void waitu(int32_t pid);
uint8_t keyboard_readu(void);
int32_t get_total_memoryu(void);

int32_t memcmp_u(void *src1, void *src2, size_t size);
void *memset_u(void *buffer, char value, size_t size);
void *memcpy_u(void *dst, void *src, size_t size);

int32_t read_cmd(char *buffer);
int32_t parse_cmd(char *buffer, int32_t buffer_size);
void execute_cmd(int32_t cmd);

int32_t open_file(char *name);
int32_t read_file(int32_t fd, void *buffer, uint32_t size);
void close_file(int32_t fd);
int32_t get_file_size(int32_t fd);

int32_t fork(void);
void exec(char *name);
int32_t read_root_directory(void *buffer);


#endif