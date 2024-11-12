#ifndef FAT_16_H
#define FAT_16_H

#include "stdint.h"

#define FS_BASE         0x30000000 // base address of os.img

// BPB = Bios parameter block
typedef struct __attribute__((packed)) {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_count;
    uint16_t root_entry_count;
    uint16_t sector_count;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sector_count;
    uint32_t large_sector_count;
    uint8_t drive_number;
    uint8_t flags;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system[8];
} BPB;

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

// file control block
typedef struct FCB {
    char name[8]; // file name
    char ext[3];  // file extention
    uint32_t cluster_index; // starting cluster index
    uint32_t dir_index; // the root directory entry index
    uint32_t file_size;
    int32_t count; // counter of open files
} FCB;

// file descriptor
typedef struct FileDesc {
    FCB *fcb; // pointer to FCB when the last time the process reads or writes in the file
    uint32_t position; // stores the current location in the file (in bytes), start from 0 when read/write
    int32_t count; // counter for the same file when forking process
} FileDesc;


typedef enum {
    FS_PHYSICAL_ADDR,
    FS_VIRTUAL_ADDR
} FsAddrType;

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

void init_fs(void);

#endif