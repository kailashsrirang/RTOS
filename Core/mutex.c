#include "mutex.h"
#include <stdint.h>
#include <stddef.h>
#include "os_kernel.h"

#define OS_NO_OWNER 0xFFFFFFFFUL

extern volatile uint32_t osCurrentTask;

void osMutexInit(Mutex_t *mutex)
{
    mutex->locked = 0;
    mutex->ownerTask = OS_NO_OWNER;
}

/* Atomic try-lock using exclusive monitor */
static uint8_t _mutexTryLock(volatile uint8_t *lock)
{
    uint32_t result;
    uint32_t tmp; /* Store inital lock value */

    /*Example*/
    // __asm__ volatile (
    //     "CODE"          /* The Assembly instructions */
    //     : [outputs]     /* 1st Colon: Where results go (C variables) */
    //     : [inputs]      /* 2nd Colon: Where data comes from (C variables) */
    //     : "clobbers"    /* 3rd Colon: The "Warning" or "Clobber" list */
    // );

    /* STREX returns 0 on success, 1 on failure */
    /* We invert: return 1 on success, 0 on failure */

    __asm volatile(
        "LDREXB  %[tmp], [%[lock]]       \n" /* Load exclusive */
        "CMP    %[tmp], #0              \n"  /* Unlocked? */
        "BNE    1f                      \n"  /* No  */
        "MOV    %[tmp], #1              \n"
        "STREXB  %[res], %[tmp], [%[lock]]\n" /* Try to store */
        "B      2f                      \n"
        "1:                             \n"
        "MOV    %[res], #1              \n" /* Failed — res = 1 (nonzero = fail) */
        "2:                             \n"
        "DMB                            \n" /* Data memory barrier */
        : [res] "=&r"(result), [tmp] "=&r"(tmp)
        : [lock] "r"(lock)
        : "cc", "memory"); /*My assebly code is goign to the change the status flags (N,Z,V,C). Do not assyme they stay the same*/
    return (result == 0) ? 1 : 0;
}

void osMutexAcquire(Mutex_t *mutex)
{
    /* busy wait*/
    while (!_mutexTryLock(&(mutex->locked)))
    {
        osTaskDelay(1); /* wait 1 sec */
    }
    mutex->ownerTask = osCurrentTask;
}

void osMutexRelease(Mutex_t *mutex)
{
    if (mutex->ownerTask != osCurrentTask)
    {
        return;
    }
    mutex->ownerTask = OS_NO_OWNER;
    __asm volatile("DMB" ::: "memory"); /* Ensure all previous memory writes are visible before the mutex is unlocked  */

    mutex->locked = 0;
}