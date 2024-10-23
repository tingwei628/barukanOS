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