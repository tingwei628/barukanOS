#include "file.h"
#include "impl.h"
#include "../../kernel/debug.h"
#include "../../kernel/mem.h"
#include "../../kernel/print.h"
#include "stddef.h"

FCB *fcb_table;
FileDesc *file_desc_table;

bool split_path(char *path, char *name, char *ext)
{
    int32_t i, j;

    for (i = 0; i < 8 && path[i] != '.' && path[i] != '\0'; i++)
    {
        // not support sub folders here
        if (path[i] == '/')
        {
            return false;
        }

        name[i] = path[i];
    }

    name[i] = '\0';

    if (path[i] == '.')
    {
        i++;
        
        for (j = 0; j < 3 && path[i] != '\0'; i++, j++)
        {
            // not support sub folders here
            if (path[i] == '/')
            {
                return false;
            }

            ext[j] = path[i];
        }

        ext[j] = '\0';
    }

    if (path[i] != '\0')
    {        
        return false;
    }

    return true;
}

// remove spaces from end of the source string
static void trim_end(char *source, char *dst, int32_t size)
{
    int32_t i;
    for (i = 0; i < size && source[i] != '\0' && source[i] != ' '; i++)
    {
        dst[i] = source[i];
    }
    dst[i] = '\0';
}

static uint32_t get_fcb(uint32_t index)
{
    DirEntry *dir_table;

    if (fcb_table[index].count == 0)
    {        
        dir_table = get_root_directory(FS_VIRTUAL_ADDR);
        fcb_table[index].dir_index = index;
        fcb_table[index].file_size = dir_table[index].file_size;
        fcb_table[index].cluster_index = dir_table[index].cluster_index;
        kmemcpy(&fcb_table[index].name, &dir_table[index].name, 8);
        kmemcpy(&fcb_table[index].ext, &dir_table[index].ext, 3);
    }
    
    // increment counter
    fcb_table[index].count++;

    return index;
}
static void put_fcb(FCB *fcb)
{
    ASSERT(fcb->count > 0);
    // decrement counter
    fcb->count--;
}

int32_t open_file(Process *proc, char *path_name)
{
    // process -> file descriptor table -> FCB table

    int32_t fd = -1; // file descriptor 
    int32_t file_desc_index = -1; // file descriptor table index
    uint32_t entry_index; // root directory entry index
    uint32_t fcb_index; // fcb table index
    
    // 100 is from FileDesc *file[100] of process
    for (int32_t i = 0; i < 100; i++)
    {
        if (proc->file[i] == NULL)
        {
            fd = i;
            break;
        }
    }

    if (fd == -1)
    {
        return -1;
    }

    for (int32_t i = 0; i < PAGE_SIZE / sizeof(FileDesc); i++)
    {
        if (file_desc_table[i].fcb == NULL)
        {
            file_desc_index = i;
            break;
        }
    }

    if (file_desc_index == -1)
    {
        return -1;
    }

    // get root directory entry index
    entry_index = search_file(path_name, FS_VIRTUAL_ADDR);
    if (entry_index == UINT32_MAX)
    {
        return -1;
    }
    
    fcb_index = get_fcb(entry_index);
    
    kmemset(&file_desc_table[file_desc_index], 0, sizeof(FileDesc));
    file_desc_table[file_desc_index].fcb = &fcb_table[fcb_index];
    file_desc_table[file_desc_index].count = 1;
    proc->file[fd] = &file_desc_table[file_desc_index];
    
    return fd;
}

static uint32_t read_raw_data(uint32_t cluster_index, char *buffer, uint32_t position, uint32_t size)
{
    uint32_t read_size = 0;
    uint32_t index = cluster_index;
    uint32_t cluster_size = get_cluster_size(FS_VIRTUAL_ADDR); 
    uint32_t count = position / cluster_size;
    uint32_t offset = position % cluster_size;

    // get next cluster index of current position
    for (uint32_t i = 0; i < count; i++)
    {
        // get next cluster index
        index = get_cluster_value(index, FS_VIRTUAL_ADDR);
        /*
            0x0000 = Cluster is free, not allocated to any file/directory
            0x0002 – 0xFFEF  = used, next cluster in file
            0xFFF7 = bad cluster
        */
        // < 0xfff7 is free or used in cluster
        ASSERT(index < 0xfff7);
    }

    BPB *bpb =  get_fs_bpb(FS_VIRTUAL_ADDR);
    char *data;

    // compare size and offset
    if (offset != 0)
    {
        read_size = (offset + size) <= cluster_size ? size : (cluster_size - offset);
        // data region
        data = (char*)((uint64_t)bpb + get_cluster_offset(index, FS_VIRTUAL_ADDR));
        kmemcpy(buffer, data + offset, read_size);
        buffer += read_size;
        index = get_cluster_value(index, FS_VIRTUAL_ADDR);
    }

    // when read_size = cluster_size - offset
    // < 0xfff7 is free or used in cluster
    while (read_size < size && index < 0xfff7)
    {
         // data region
        data = (char*)((uint64_t)bpb + get_cluster_offset(index, FS_VIRTUAL_ADDR));
   
        // if cluster_size >= remaining data
        // since remaining data = size - read_size, then
        // cluster_size >= size - read_size, then
        // cluster_size + read_size >= size
        if (read_size + cluster_size >= size)
        {
            kmemcpy(buffer, data, size - read_size);
            read_size = size;
            break;
        }
              
        kmemcpy(buffer, data, cluster_size);     
        buffer += cluster_size;
        read_size += cluster_size;
        // get next cluster index
        index = get_cluster_value(index, FS_VIRTUAL_ADDR);
    }
    
    return read_size;
}

int32_t read_file(Process *proc, int32_t fd, void *buffer, uint32_t size)
{
    uint32_t position = proc->file[fd]->position;
    uint32_t file_size = proc->file[fd]->fcb->file_size;
    uint32_t read_size;

    // positon = current position of reading in file
    // size = we want to read
    if (position + size > file_size)
    {
        return -1;
    }

    read_size = read_raw_data(proc->file[fd]->fcb->cluster_index, buffer, position, size);
    proc->file[fd]->position += read_size;
    
    return read_size;
}

void close_file(Process *proc, int32_t fd)
{
    put_fcb(proc->file[fd]->fcb);
    
    proc->file[fd]->count--;

    if (proc->file[fd]->count == 0)
    {
        proc->file[fd]->fcb = NULL;
    }
    proc->file[fd] = NULL;
}

uint32_t get_file_size(Process *proc, int32_t fd)
{
    return proc->file[fd]->fcb->file_size;
}

BPB* get_fs_bpb(FsAddrType fs_addr_type)
{
    // 0x1be = 446
    if (fs_addr_type == FS_PHYSICAL_ADDR)
    {
        // lba of partitionon = 0x3f (ref: boot.asm)
        uint32_t lba = *(uint32_t*)((uint64_t)FS_BASE + 0x1be + 8);
        // ((uint64_t)FS_BASE + lba * 512) = physical address of bios parameter block
        return (BPB*)((uint64_t)FS_BASE + lba * 512);
    }

    if (fs_addr_type == FS_VIRTUAL_ADDR)
    {
        // lba of partitionon = 0x3f (ref: boot.asm)
        // virtual address of base address of os.img = P2V(FS_BASE)
        uint32_t lba = *(uint32_t*)(P2V(FS_BASE) + 0x1be + 8);
        return (BPB*)P2V(FS_BASE + lba * 512);
    }

    return NULL;
    
}

uint32_t get_cluster_offset(uint32_t index, FsAddrType fs_addr_type)
{
    uint32_t res_size;
    uint32_t fat_size;
    uint32_t dir_size;

    ASSERT(index >= 2);

    BPB* p = get_fs_bpb(fs_addr_type);

    res_size = (p->reserved_sector_count) * (p->bytes_per_sector);
    fat_size = (p->fat_count) * (p->sectors_per_fat) * (p->bytes_per_sector);
    dir_size = p->root_entry_count * sizeof(DirEntry);

    // (index - 2) since start from cluster 2
    return res_size + fat_size + dir_size + (index - 2) * ((p->sectors_per_cluster) * (p->bytes_per_sector));
}

uint16_t* get_fat_table(FsAddrType fs_addr_type)
{
    BPB *p = get_fs_bpb(fs_addr_type);
    uint32_t offset = (p->reserved_sector_count) * (p->bytes_per_sector);

    return (uint16_t*)((uint8_t*)p + offset);
}

uint16_t get_cluster_value(uint32_t cluster_index, FsAddrType fs_addr_type)
{
    uint16_t *fat_table = get_fat_table(fs_addr_type);

    return fat_table[cluster_index];
}

uint32_t get_cluster_size(FsAddrType fs_addr_type)
{
   BPB *bpb = get_fs_bpb(fs_addr_type);

   return (uint32_t)bpb->bytes_per_sector * bpb->sectors_per_cluster;
}

uint32_t get_root_directory_count(FsAddrType fs_addr_type)
{
   BPB *bpb = get_fs_bpb(fs_addr_type);

   return bpb->root_entry_count;
}

DirEntry *get_root_directory(FsAddrType fs_addr_type)
{
    BPB  *p; 
    uint32_t offset; 

    p = get_fs_bpb(fs_addr_type);
    offset = (p->reserved_sector_count + p->fat_count * p->sectors_per_fat) * p->bytes_per_sector;

    return (DirEntry*)((uint8_t*)p + offset);
}

int32_t read_root_directory(char *buffer)
{
    DirEntry *dir_entry = get_root_directory(FS_VIRTUAL_ADDR);
    uint32_t count = get_root_directory_count(FS_VIRTUAL_ADDR);
    
    kmemcpy(buffer, dir_entry, count * sizeof(DirEntry));
        
    return count;
}

bool is_file_name_equal(DirEntry *dir_entry, char *name, char *ext)
{
    bool status = false;

    char dir_entry_name[9] = {0};   // 8 + 1 ('\0')
    char dir_entry_ext[4] = {0};    // 3 + 1 ('\0')

    trim_end(dir_entry->name, dir_entry_name, 8);
    trim_end(dir_entry->ext, dir_entry_ext, 3);
    
    if (kmemcmp(dir_entry_name, name, 8) == 0 && kmemcmp(dir_entry_ext, ext, 3) == 0)
    {
        status = true;
    }

    return status;
}

// search file and return root directory entry index
uint32_t search_file(char *path, FsAddrType fs_addr_type)
{
    char name[9] = {0}; // 8 + 1 ('\0')
    char ext[4] = {0};  // 3 + 1 ('\0')

    uint32_t root_entry_count;
    DirEntry *dir_entry; 

    bool status = split_path(path, name, ext);

    if (status)
    {
        root_entry_count = get_root_directory_count(fs_addr_type);
        dir_entry = get_root_directory(fs_addr_type);
        
        for (uint32_t i = 0; i < root_entry_count; i++)
        {
            // check file allocation status
            if (dir_entry[i].name[0] == EMPTY || dir_entry[i].name[0] == DELETED)
                continue;

            // if Long File Name
            if (dir_entry[i].attributes == LONG_FILE_NAME)
                continue;
            

            if (is_file_name_equal(&dir_entry[i], name, ext))
                return i;
            
        }
    }

    return UINT32_MAX;
}