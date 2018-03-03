#include <osapi.h>
#include <mem.h>


void operator delete(void * ptr);
__extension__ typedef int __guard __attribute__((mode (__DI__)));
extern "C" int  __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release(__guard *);
extern "C" void __cxa_guard_abort(__guard *);
extern "C" void __cxa_pure_virtual(void);


#if defined(__xtensa__)
extern "C" void abort()
{
    while (1);
}
#endif


void* operator new(size_t size)
{
    //os_printf("new %d\n", size);
    return(os_malloc(size));
}

void* operator new[](size_t size)
{
    //os_printf("new[] %d\n", size);
    return(os_malloc(size));
}

void operator delete(void *ptr)
{
    if (ptr) os_free(ptr);
}

void operator delete[](void *ptr)
{
    if (ptr) os_free(ptr);
}


int __cxa_guard_acquire(__guard *g)
{
    return(!*(char *)(g));
}

void __cxa_guard_release(__guard *g)
{
    *(char *)g = 1;
}

void __cxa_guard_abort(__guard *)
{
#if !defined(__AVR__)
    abort();
#endif
}

void __cxa_pure_virtual(void)
{
#if !defined(__AVR__)
    abort();
#endif
}

void __cxa_deleted_virtual(void)
{
#if !defined(__AVR__)
    abort();
#endif
}
