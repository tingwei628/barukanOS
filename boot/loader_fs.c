#include "../kernel/mem.h"
#include "../kernel/print.h"
#include "../fs/fat16/file.h"
#include "../fs/fat16/impl.h"
#include "../kernel/debug.h"
#include "loader_fs.h"
#include "stddef.h"

static uint32_t read_raw_data(uint32_t cluster_index, char *buffer, uint32_t size)
{
   BPB* bpb;
   char *data;
   uint32_t read_size = 0;
   uint32_t cluster_size; 
   uint32_t index; 
   
   bpb = get_fs_bpb(FS_PHYSICAL_ADDR);
   cluster_size = get_cluster_size(FS_PHYSICAL_ADDR);
   index = cluster_index;

   // since cluster index should start from 2
   if (index < 2)
   {
      return UINT32_MAX;
   }
   
   while (read_size < size)
   {
      // get current data
      data  = (char *)((uint64_t)bpb + get_cluster_offset(index, FS_PHYSICAL_ADDR));
      // get next cluster index
      index = get_cluster_value(index, FS_PHYSICAL_ADDR);
      
      // if next cluster is bad cluster, last cluster
      // 0xfff7 = bad cluster
      // > 0xfff7 = last cluster (end of file)
      if (index >= 0xfff7)
      {
         // size of remaining data = size - read_size
         kmemcpy(buffer, data, size - read_size);
         read_size += size - read_size;
         break;
      }

      kmemcpy(buffer, data, cluster_size);

      buffer += cluster_size;
      read_size += cluster_size;
   }

   return read_size;
}

static uint32_t read_loader_file(uint32_t cluster_index, void *buffer, uint32_t size)
{
    return read_raw_data(cluster_index, buffer, size);
}

// load file rom path into addr 
int32_t load_file(char *path, uint64_t addr)
{
   uint32_t index;
   uint32_t file_size;
   uint32_t cluster_index;
   DirEntry *dir_entry;
   int32_t ret = -1;
   
   // index = roor directory entry index
   index = search_file(path, FS_PHYSICAL_ADDR);
   if (index != UINT32_MAX)
   {
      dir_entry = get_root_directory(FS_PHYSICAL_ADDR);
      file_size = dir_entry[index].file_size;
      cluster_index = dir_entry[index].cluster_index; // start cluster index
      
      if (read_loader_file(cluster_index, (void*)addr, file_size) == file_size)
      {
         ret = 0;
      }
   }

   return ret;
}

void init_loader_fs(void)
{
   uint8_t *p = (uint8_t*)get_fs_bpb(FS_PHYSICAL_ADDR);   
   // 0x1fe = 510
   // 0x1ff = 511
   if (p[0x1fe] != 0x55 || p[0x1ff] != 0xaa)
   {
      printk("invalid signature\n");
      ASSERT(0);
   }
}