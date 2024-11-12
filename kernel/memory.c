#include "mem.h"
#include "memory.h"
#include "print.h"
#include "debug.h"

static FreeMemRegion physical_free_mem_region[50]; // free mem in physical memory
static Page virtual_free_memory; // page list of free memory in virtual memory
static uint64_t total_mem; // total physical free memory (bytes)
extern char end; // end symbol of kernel from linker script

static void set_virtual_free_memory_region(uint64_t v, uint64_t e)
{
    for (uint64_t start = PA_UP(v); start + PAGE_SIZE <= e; start += PAGE_SIZE)
    {
        if (start + PAGE_SIZE <= 0xffff800030000000)
        {          
            kfree(start);
        }
    }
}

uint64_t init_memory(void)
{  
    int32_t count = *(int32_t*)MEMORY_ENTRY_NUMBER_ADDR;
    E820 *mem_map = (E820*)MEMORY_MAP_ADDR;

    int32_t free_region_count = 0;

    ASSERT(count <= 50);

	for(int32_t i = 0; i < count; i++)
    {        
        if(mem_map[i].type == E820_TYPE_RAM)
        {		
            physical_free_mem_region[free_region_count].address = mem_map[i].address;
            physical_free_mem_region[free_region_count].length = mem_map[i].length;
            total_mem += mem_map[i].length;
            free_region_count++;
        }
        //printk("%x  %uKB  %u\n",mem_map[i].address, mem_map[i].length/1024, (uint64_t)mem_map[i].type);
	}

    //printk("Total memory is %uMB\n", total_mem/1024/1024);

    for (int32_t i = 0; i < free_region_count; i++)
    {                  
        uint64_t vstart = P2V(physical_free_mem_region[i].address);
        uint64_t vend = vstart + physical_free_mem_region[i].length;

        // &end is kenerl end address
        if (vstart > (uint64_t)&end)
        {
            set_virtual_free_memory_region(vstart, vend);
        }
        else if (vend > (uint64_t)&end)
        {
            set_virtual_free_memory_region((uint64_t)&end, vend);
        }       
    }
    uint64_t virtual_free_memory_end = (uint64_t)virtual_free_memory.next + PAGE_SIZE;
    return virtual_free_memory_end;
}

// total physical free memory (MB)
uint64_t get_total_memory(void)
{
    return total_mem/1024/1024;
}

// add page to page list
void kfree(uint64_t v)
{
    ASSERT(v % PAGE_SIZE == 0);
    ASSERT(v >= (uint64_t)&end);
    ASSERT(v + PAGE_SIZE <= 0xffff800030000000);

    // v is start address of new page
    Page *page_start_address = (Page*)v;

    // virtual_free_memory is dummy node which points to head of page list  
    // virtual_free_memory.next is head of page list
    // add page_start_address in the front of head
    page_start_address->next = virtual_free_memory.next;

    // v is new head of page list
    // virtual_free_memory.next is still head of page list 
    virtual_free_memory.next = page_start_address;

}

// remove page from page list
void* kmalloc(void)
{
    // page_address is head of page list
    Page *page_address = virtual_free_memory.next;

    if (page_address != NULL)
    {
        ASSERT((uint64_t)page_address % PAGE_SIZE == 0);
        ASSERT((uint64_t)page_address >= (uint64_t)&end);
        ASSERT((uint64_t)page_address + PAGE_SIZE <= 0xffff800030000000);

        // remove head of page list
        // head to next page
        // virtual_free_memory.next is still head of page list 
        virtual_free_memory.next = page_address->next;        
    }
    
    return page_address;
}

