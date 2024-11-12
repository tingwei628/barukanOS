#include "stdint.h"
#include "stddef.h"

// copy from kmemcmp
int32_t memcmp_u(void *src1, void *src2, size_t size)
{

    char *d = (char*)src1, *s = (char*)src2;

    while (size > 0)
    {
        if (*d > *s) return 1;
        if (*d < *s) return -1;
        
        d++;
        s++;

        size--;
    }

    return 0;
}

// copy from kmemset
void *memset_u(void *buffer, char value, size_t size)
{
    uint8_t *ptr = (uint8_t*)buffer;

    while (size > 0)
    {
        *ptr = value;
        ptr++;
        size--;
    }

    return buffer;
}

// copy from kmemcpy
void *memcpy_u(void *dst, void *src, size_t size)
{
    uint8_t *d = (uint8_t*)dst, *s = (uint8_t*)src;
    while (size > 0)
    {
        *d = *s;
        d++;
        s++;
        size--;
    }

    return dst;

} 