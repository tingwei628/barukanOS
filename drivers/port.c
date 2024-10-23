#include "port.h"


// read a byte from port
uint8_t in_byte(uint16_t port)
{
    //inb port, byte_saved_at_reg
    uint8_t result;
    __asm__ __volatile__ (
        "inb %1, %0"
        : "=a" (result)
        : "d" (port)
        :
    );
    return result;
}