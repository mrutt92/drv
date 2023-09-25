#include <string.h>

/**
 * print_int.c
 */
static inline void print_int(long x)
{
    *(volatile long*)0xFFFFFFFFFFFF0000 = x;
}

static inline void print_hex(unsigned long x)
{
    *(volatile unsigned long*)0xFFFFFFFFFFFF0008 = x;
}

static inline void print_char(char x)
{
    *(volatile char*)0xFFFFFFFFFFFF0010 = x;
}

#define ARRAY_SIZE(x) \
    (sizeof(x)/sizeof((x)[0]))


int main()
{
    void *sp;
    asm volatile ("mv %0, sp" : "=r" (sp));
    print_hex((unsigned long)sp);
    return 0;
}
