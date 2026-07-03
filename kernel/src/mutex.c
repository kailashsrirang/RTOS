#include "mutex.h"
#include <stdint.h>
#include <stddef.h>
#include "os_kernel.h"
#include <string.h>

#define ICSR (*(volatile uint32_t *)0xE000ED04UL)

extern volatile uint32_t osCurrentTask;
extern volatile TCB_t _tcbs[OS_MAX_TASKS];

void osMutexInit(Mutex_t *mutex)
{
    /* Initailze mutex queue*/
    mutex->waitCount = 0;
    mutex->waitHead = 0;
    mutex->waitTail = 0;
    memset(&mutex->waitQueue, 0, sizeof(mutex->waitQueue));

    /* Initailze mutex */
    mutex->locked = 0;
    mutex->ownerTask = MUTEX_NO_OWNER;
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
    __asm volatile("CPSID I");

    if (!_mutexTryLock(&(mutex->locked)))
    {
        if (!_mutexEnqueue(mutex, osCurrentTask))
        {
            __asm volatile("CPSIE I");
            return;
        }
        _tcbs[osCurrentTask].state = TASK_BLOCKED;
        ICSR = (1U << 28); /* PendSV set pending */
    }
    else
    {
        mutex->ownerTask = osCurrentTask;
    }
    __asm volatile("CPSIE I");
}

void osMutexRelease(Mutex_t *mutex)
{
    __asm volatile("CPSID I");

    if (mutex->ownerTask != osCurrentTask)
    {
        __asm volatile("CPSIE I");
        return;
    }

    uint32_t nextOwner = _mutexDequeue(mutex);
    if (nextOwner != MUTEX_NO_OWNER)
    {
        _tcbs[nextOwner].state = TASK_READY;
        mutex->ownerTask = nextOwner;
        ICSR = (1U << 28); /* PendSV set pending */
        __asm volatile("CPSIE I");
        return;
    }
    else
    {
        mutex->ownerTask = MUTEX_NO_OWNER;
        __asm volatile("DMB" ::: "memory"); /* Ensure all previous memory writes are visible before the mutex is unlocked  */

        mutex->locked = 0;
    }
    ICSR = (1U << 28); /* PendSV set pending */
    __asm volatile("CPSIE I");
}

uint8_t _mutexEnqueue(Mutex_t *mutex, uint32_t taskIndex)
{
    if (mutex->waitCount >= MUTEX_QUEUE_SIZE)
    {
        return 0;
    }

    mutex->waitQueue[mutex->waitTail] = taskIndex;
    mutex->waitTail = (mutex->waitTail + 1) % MUTEX_QUEUE_SIZE;
    mutex->waitCount++;

    return 1;
}
uint32_t _mutexDequeue(Mutex_t *mutex)
{
    if (mutex->waitCount == 0)
    {
        return MUTEX_NO_OWNER;
    }

    uint32_t newTaskIndex = mutex->waitQueue[mutex->waitHead];

    mutex->waitHead = (mutex->waitHead + 1) % MUTEX_QUEUE_SIZE;
    mutex->waitCount--;

    return newTaskIndex;
}
