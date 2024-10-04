#include "memory.h"
#include "vm.h"
#include "debug.h"
#include "idt.h"
#include "print.h"

static void free_pdt(uint64_t map)
{
    PDPTR *pml4t_base = (PDPTR*)map;

    // each pml4 table has 512 pml4 entries
    for (int32_t i = 0; i < 512; i++)
    {
        // if this pml4 entry is valid and point to next level table
        if ((uint64_t)pml4t_base[i] & PTE_P)
        {            
            PD *pdpt_base = (PD*)P2V(PDE_ADDR(pml4t_base[i]));
            
            // each pdp table has 512 pdp entries
            for (int32_t j = 0; j < 512; j++)
            {
                // if this pdp entry is valid and point to next level table
                if ((uint64_t)pdpt_base[j] & PTE_P)
                {
                    // free each pd table       
                    // PDE_ADDR(pdpt_base[j]) to get pd base address (physical address)
                    // P2V(PDE_ADDR(pdpt_base[j])) to get pd base address (virtual address)
                    kfree(P2V(PDE_ADDR(pdpt_base[j])));
                    // reset pdp entry to 0
                    pdpt_base[j] = 0;
                }
            }
        }
    }
}

static void free_pdpt(uint64_t map)
{
    PDPTR *pml4t_base = (PDPTR*)map;

    // each pml4 table has 512 pml4 entries
    for (int32_t i = 0; i < 512; i++)
    {
        // if this pml4 entry is valid and point to next level table
        if ((uint64_t)pml4t_base[i] & PTE_P)
        {   
            // free each pdp table       
            // PDE_ADDR(pml4t_base[i]) to get pdp base address (physical address)
            // P2V(PDE_ADDR(pml4t_base[i])) to get pdp base address (virtual address)
            kfree(P2V(PDE_ADDR(pml4t_base[i])));
            // reset pml4t entry to 0
            pml4t_base[i] = 0;
        }
    }
}

static void free_pml4t(uint64_t map)
{
    kfree(map);
}

// map is pml4 table base address (virtual address)
// v is start of virtual address
// is_alloc_page is to create new page
// physical_address is start of mapping of physical address
// attribute is attribute of entry
static PDPTR get_pdpt_base_from_pml4t(uint64_t map, uint64_t v, bool is_alloc_page, uint32_t attribute)
{
    PDPTR *pml4t_base =  (PDPTR*)map;   // point to pml4 table base address
    PDPTR pdpt_base = NULL;             // point to pdp table base address
    
    // get pml4 index from vtsart
    uint32_t index = (v >> 39) & 0x1ff;

    // check if this pml4 entry of vstart is valid and point to next level table
    if ((uint64_t)pml4t_base[index] & PTE_P)
    {
        // PDE_ADDR(map_entry[index]) to get physical address of pdpt base address
        pdpt_base = (PDPTR)P2V(PDE_ADDR(pml4t_base[index]));
        return pdpt_base;
    }

    // vstart is NOT in this page (map), create new page
    if (is_alloc_page)
    {
        // pdpt_base = new page in virtual memory
        pdpt_base = (PDPTR)kmalloc();          
        if (pdpt_base != NULL)
        { 
            // reset new page    
            kmemset(pdpt_base, 0, PAGE_SIZE);
            // set new pml4 entry
            // V2P(pdpt_base) to get physical address of page
            pml4t_base[index] = (PDPTR)(V2P(pdpt_base) | attribute);           
        }
    } 

    return pdpt_base;    
}

// map is pml4 table base address (virtual address)
// v is start of virtual address
// is_alloc_page is to create new page
// physical_address is start of mapping of physical address
// attribute is attribute of entry
static PD get_pdt_base_from_pdpt(uint64_t map, uint64_t v, bool is_alloc_page, uint32_t attribute)
{
    PDPTR pdpt_base = NULL; // point to pdp table base address
    PD pdt_base = NULL;
    // get pdp index from vtsart
    uint32_t index = (v >> 30) & 0x1ff;

    pdpt_base = get_pdpt_base_from_pml4t(map, v, is_alloc_page, attribute);
    if (pdpt_base == NULL)
        return NULL;
    
    // check if this pdp entry of vstart is valid and point to next level table
    if ((uint64_t)pdpt_base[index] & PTE_P)
    {   
        // PDE_ADDR(pdpt_base[index]) to get physical address of pdt base address
        pdt_base = (PD)P2V(PDE_ADDR(pdpt_base[index]));        
        return pdt_base;  
    }

    // is_alloc_page = true, create new page when page does not exist
    if (is_alloc_page)
    {
        // pdt_base = new page in virtual memory
        pdt_base = (PD)kmalloc();
        if (pdt_base != NULL)
        {    
            kmemset(pdt_base, 0, PAGE_SIZE);       
            pdpt_base[index] = (PD)(V2P(pdt_base) | attribute);
        }
    } 

    return pdt_base;
}

// map is pml4 table base address (virtual address)
// v is start of virtual address
// e is end of virtual address
// physical_address is start of mapping of physical address
// attribute is attribute of entry
bool map_pages(uint64_t map, uint64_t v, uint64_t e, uint64_t physical_address, uint32_t attribute)
{
    uint64_t vstart = PA_DOWN(v);   // get previous aligned address of start 
    uint64_t vend = PA_UP(e);       // get next aligned address of end
    PD pdt_base = NULL;
    uint32_t index;

    ASSERT(v < e);
    ASSERT(physical_address % PAGE_SIZE == 0);
    ASSERT(physical_address + vend - vstart <= 1024 * 1024 * 1024); // 1024 * 1024 * 1024 = 1gb

    // set PD entry to pdt
    do
    {
        // pdt_base is virtual memory
        pdt_base = get_pdt_base_from_pdpt(map, vstart, true, attribute);
        if (pdt_base == NULL)
        {
            return false;
        }
        
        // because page start from vstart
        // get pd index from vstart
        index = (vstart >> 21) & 0x1ff;
        // because check if this pd entry of vstart is valid and point to next level table
        // bit-0 = 0 means it is not used, so we can set this pd entry
        ASSERT(((uint64_t)pdt_base[index] & PTE_P) == 0);

        // index is page offset
        // pdt_base[index] equals to *(pd + index)
        // pdt_base[index] = pde entry value at address of (pdt_base + index)
        // PTE_ENTRY is set for 2MB page in pd entry
        pdt_base[index] = (uint64_t)(physical_address | attribute | PTE_ENTRY);

        vstart += PAGE_SIZE;            // virtual address start of next pd entry 
        physical_address += PAGE_SIZE;  // physical address of next pd entry 
    } while (vstart + PAGE_SIZE <= vend);
  
    return true;
}

void switch_vm(uint64_t map)
{
    // save pml4 table base address (physical address) to cr3
    load_cr3(V2P(map));   
}

// setup virtual memory for kernel space
uint64_t setup_kvm(uint64_t virtual_free_memory_end)
{
    bool status = false;

    // get a page from page list of free memory in virtual memory
    // page_map is pml4 table base address (virtual address)
    uint64_t page_map = (uint64_t)kmalloc();

    if (page_map != 0)
    {
        kmemset((void*)page_map, 0, PAGE_SIZE);        
        
        // check each page level of memory map
        status = map_pages(page_map, KERNEL_VIRTUAL_MEMORY_START, virtual_free_memory_end, V2P(KERNEL_VIRTUAL_MEMORY_START), PTE_P | PTE_W);
        if (status == false)
        {
            free_vm(page_map);
            page_map = 0;
        }
    }
    return page_map;
}

void init_kvm(uint64_t virtual_free_memory_end)
{
    // page_map is pml4 table base address (virtual address)
    uint64_t page_map = setup_kvm(virtual_free_memory_end);
    ASSERT(page_map != 0);
    switch_vm(page_map);
    printk("memory manager is working now");
}

// setup virtual memory for user space
// map is pml4 table base address (virtual address)
bool setup_uvm(uint64_t map, uint64_t start, int32_t size)
{
    bool status = false;

    // alloc a page to program of user space
    void *page = kmalloc();

    if (page != NULL)
    {
        kmemset(page, 0, PAGE_SIZE);
        // 0x400000 is base address (virtual memory) of program of user space
        // 0x400000 + PAGE_SIZE is end of address (virtual memory) of program of user space
        // set PTE_U because it is in use space
        status = map_pages(map, 0x400000, 0x400000 + PAGE_SIZE, V2P(page), PTE_P | PTE_W | PTE_U);
        if (status)
        {
            // copy program instruction and data into this page
            kmemcpy(page, (void*)start, size);
        }
        else
        {
            kfree((uint64_t)page);
            free_vm(map);
        }
    }
    
    return status;
}


void free_pages(uint64_t map, uint64_t vstart, uint64_t vend)
{
    uint32_t index; 

    ASSERT(vstart % PAGE_SIZE == 0);
    ASSERT(vend % PAGE_SIZE == 0);

    do
    {
        // reset attribute to 0
        PD pdt_base = get_pdt_base_from_pdpt(map, vstart, false, 0);

        if (pdt_base != NULL)
        {
            // get pd index from vstart
            index = (vstart >> 21) & 0x1ff;
            
            // check if this pd entry of vstart is valid and point to next level table
            if (pdt_base[index] & PTE_P)
            {
                // free each page table       
                // PTE_ADDR(pdt_base[index]) to get page frame address (physical address)
                // P2V(PTE_ADDR(pdt_base[index])) to get page frame address (virtual address)
                kfree(P2V(PTE_ADDR(pdt_base[index])));
                // reset pd entry to 0
                pdt_base[index] = 0;
            }
        }

        vstart += PAGE_SIZE;
    } while (vstart + PAGE_SIZE <= vend);
}

void free_vm(uint64_t map)
{   
    // 0x400000 is base address (virtual memory) of program of user space
    // 0x400000 + PAGE_SIZE is end of address (virtual memory) of program of user space
    free_pages(map, 0x400000, 0x400000 + PAGE_SIZE);
    free_pdt(map);
    free_pdpt(map);
    free_pml4t(map);
}