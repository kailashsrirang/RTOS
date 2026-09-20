#include "mem.h"
#include <stdint.h>

void *memset(void *dest, int value, size_t len)
{
    unsigned char *ptr = (unsigned char *)dest;

    while (len--)
    {
        *ptr++ = (unsigned char)value;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    while (n--)
    {
        *d++ = *s++;
    }

    return dest;
}