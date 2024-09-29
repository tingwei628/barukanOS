#include "debug.h"
#include "print.h"

void error_check(uint8_t *file, uint64_t line)
{
    printk("\n------------------------------------------\n");
    printk("             ERROR CHECK");
    printk("\n------------------------------------------\n");
    printk("Assertion Failed [%s:%u]", file, line);

    while (1) { }
}