// mem.h
#ifndef MEM_H
#define MEM_H
#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int snprintf(char *str, size_t size, const char *format, ...);

#endif
