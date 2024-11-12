#ifndef LOADER_FS_H
#define LOADER_FS_H

#include "stdint.h"

int32_t load_file(char *path, uint64_t addr);
void init_loader_fs(void);

#endif