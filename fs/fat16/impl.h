#ifndef FAT_16_IMPL_H
#define FAT_16_IMPL_H

#include "file.h"
#include "../../kernel/process.h"

#include "stdint.h"
#include "stdbool.h"

void close_file(Process *proc, int32_t fd);
int32_t read_file(Process *proc, int32_t fd, void *buffer, uint32_t size);
int32_t open_file(Process *proc, char *path_name);
uint32_t get_file_size(Process *proc, int32_t fd);
int32_t read_root_directory(char *buffer);

bool split_path(char *path, char *name, char *ext);
BPB* get_fs_bpb(FsAddrType fs_addr_type);
uint32_t get_cluster_offset(uint32_t index, FsAddrType fs_addr_type);
uint16_t* get_fat_table(FsAddrType fs_addr_type);
uint16_t get_cluster_value(uint32_t cluster_index, FsAddrType fs_addr_type);
uint32_t get_cluster_size(FsAddrType fs_addr_type);
uint32_t get_root_directory_count(FsAddrType fs_addr_type);
DirEntry* get_root_directory(FsAddrType fs_addr_type);
bool is_file_name_equal(DirEntry *dir_entry, char *name, char *ext);
uint32_t search_file(char *path, FsAddrType fs_addr_type);

#endif