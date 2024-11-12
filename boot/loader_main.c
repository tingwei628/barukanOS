#include "loader_fs.h"
#include "../kernel/debug.h"

void EMain(void)
{
   init_loader_fs();
   // length limit of file name is 8
   // length limit of ext is 3
   // since name and ext of DirEntry in fat16 are uppercases
   // load "KERNEL.BIN" from os.img into 0x200000
   ASSERT(load_file("KERNEL.BIN", 0x200000) == 0);
   // load "USER1.BIN" from os.img into 0x30000
   ASSERT(load_file("USER1.BIN", 0x30000) == 0);
}