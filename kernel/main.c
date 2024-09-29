#include "idt.h"
#include "print.h"
#include "debug.h"

void kernal_main(void)
{
    init_idt();
    
    uint8_t *string = "Hello barukanOS";
    printk("%s", string);
}