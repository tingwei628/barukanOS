#include "file.h"
#include "../../kernel/debug.h"
#include "../../kernel/print.h"
#include "../../kernel/mem.h"
#include "../../kernel/memory.h"
#include "stdbool.h"
#include "impl.h"

extern FCB *fcb_table;
extern FileDesc *file_desc_table;

static bool init_fcb(void)
{
    fcb_table = (FCB*)kmalloc();

    if (fcb_table == NULL)
    {
	    return false;
    }

    kmemset(fcb_table, 0, PAGE_SIZE);

    return true;
}

static bool init_file_desc(void)
{
    file_desc_table = (FileDesc*)kmalloc();

    if (file_desc_table == NULL)
    {
	    return false;
    }

    kmemset(file_desc_table, 0, PAGE_SIZE);

    return true;
}

void init_fs(void)
{
    uint8_t *p = (uint8_t*)get_fs_bpb(FS_VIRTUAL_ADDR);
    
    if (p[0x1fe] != 0x55 || p[0x1ff] != 0xaa)
    {
        printk("invalid signature\n");
        ASSERT(0);
    }
    // init FCB
    ASSERT(init_fcb());
    // init file descriptor table
    ASSERT(init_file_desc());
}

