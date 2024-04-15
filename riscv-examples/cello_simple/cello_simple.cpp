#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "cello.hpp"
#include "pandohammer/storage.h"
#include "pandohammer/addressmap.hpp"
#include "pandohammer/cpuinfo.h"
#include "pandohammer/atomic.h"
#include "pandohammer/mmio.h"
l1sp_storage(int) wait_until_not_zero;

static int my_printf(const char*fmt, ...)
{
#if 1
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    write(STDOUT_FILENO, buffer, ret);
    return ret;
#else
    return 0;
#endif
}

int CelloMain(int argc, char *argv[])
{
    int other = myCoreId() == 0 ? 1 : 0;
    int other_x = coreXFromId(other);
    int other_y = coreYFromId(other);
    int *my_ptr = std::addressof(wait_until_not_zero);
    int *other_ptr =
        virtual_address{std::addressof(wait_until_not_zero)}
        .set_core_x(coreXFromId(other))
        .set_core_y(coreYFromId(other))
        .set_is_global(true);

    my_printf("my_ptr = %p\n", my_ptr);
    my_printf("my_ptr = {is_global = %x}\n",
              (uintptr_t)virtual_address{my_ptr}.is_global());
    my_printf("other_ptr = %p\n", other_ptr);
    my_printf("other_ptr = {is_global = %x}\n",
              (uintptr_t)virtual_address{other_ptr}.is_global());
    
    // get the stack pointer
    void *p;
    asm volatile ("mv %0, sp" : "=r" (p));

    void *allocated = malloc(0x1000);
    my_printf("allocated = %p, is_l2=%x, is_global=%x\n"
              ,allocated
              ,virtual_address{other_ptr}.is_l2()
              ,(uintptr_t)virtual_address{allocated}.is_global());

    // stack
    void *sp;
    asm volatile ("mv %0, sp" : "=r" (sp));
    my_printf("sp = %p, is_l1=%x, is_global=%x\n"
              ,sp
              ,virtual_address{sp}.is_l1()
              ,(uintptr_t)virtual_address{sp}.is_global());
    return 0;
}
