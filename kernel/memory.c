#include "memory.h"

void* kmemset(void *buffer, uint8_t value, size_t size)
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
void* kmemcpy(void *dst, void *src, size_t size)
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
void* kmemmove(void *dst, void *src, size_t size)
{
    if (src < dst)
    {
        return kmemcpy(dst, src, size);
    }

    uint8_t *d = (uint8_t*)dst + size-1, *s = (uint8_t*)src + size-1;

    while (size > 0) 
    {
        *d = *s;
        d--;
        s--;
        size--; 
    }

    return dst;

}


uint8_t kmemcmp(void *src1, void *src2, size_t size)
{

    uint8_t *d = (uint8_t*)src1, *s = (uint8_t*)src2;

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

