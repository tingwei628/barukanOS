#include "debug.h"
#include "print.h"

void error_check(uint8_t *file, uint8_t const *func, uint64_t line)
{
    printk("\n------------------------------------------\n");
    printk("             ERROR CHECK");
    printk("\n------------------------------------------\n");
    printk("Assertion Failed [%s:%s:%u]", file, func, line);

    while (1) { }
}