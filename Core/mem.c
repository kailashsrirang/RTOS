#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

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

// Helper to convert integer to string
static char *itoa(int value, char *str, int base)
{
    char *rc, *ptr, *low;
    if (base < 2 || base > 36)
        return str;
    rc = ptr = str;
    if (value < 0 && base == 10)
        *ptr++ = '-';
    low = ptr;
    int v = (value < 0) ? -value : value;
    do
    {
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[v % base];
        v /= base;
    } while (v);
    *ptr-- = '\0';
    while (low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

int snprintf(char *buffer, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    size_t n = 0;
    const char *p = format;

    while (*p && n < size - 1)
    {
        if (*p == '%' && *(p + 1))
        {
            p++;
            if (*p == 'd')
            {
                char tmp[12];
                itoa(va_arg(args, int), tmp, 10);
                for (char *t = tmp; *t && n < size - 1; t++)
                    buffer[n++] = *t;
            }
            else if (*p == 'x')
            {
                char tmp[12];
                itoa(va_arg(args, int), tmp, 16);
                for (char *t = tmp; *t && n < size - 1; t++)
                    buffer[n++] = *t;
            }
            else if (*p == 's')
            {
                char *s = va_arg(args, char *);
                while (*s && n < size - 1)
                    buffer[n++] = *s++;
            }
            else
            {
                buffer[n++] = *p;
            }
        }
        else
        {
            buffer[n++] = *p;
        }
        p++;
    }
    buffer[n] = '\0';
    va_end(args);
    return n;
}
