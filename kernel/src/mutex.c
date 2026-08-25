#include "mutex.h"
#include <stdint.h>
#include <stddef.h>
#include "os_kernel.h"
#include <string.h>
#include "os_interrupt.h"

extern void osScheduler(void);
extern volatile uint32_t osCurrentTask;
extern volatile TCB_t _tcbs[OS_MAX_TASKS];

OsStatus osMutexInit(Mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    mutex->waitCount = 0U;

    mutex->waitHead = 0U;

    mutex->waitTail = 0U;

    memset(
        mutex->waitQueue,
        0,
        sizeof(mutex->waitQueue));

    mutex->locked = 0U;

    mutex->ownerTask = MUTEX_NO_OWNER;

    return OS_OK;
}

/* Atomic try-lock using exclusive monitor */
static uint8_t _mutexTryLock(volatile uint8_t *lock)
{
    uint32_t result;
    uint32_t tmp;

    __asm volatile(
        "LDREXB  %[tmp], [%[lock]]        \n"  /* Load current lock value */
        "CMP     %[tmp], #0               \n"  /* Is it unlocked (0)? */
        "BNE     1f                       \n"  /* If locked (!= 0), branch out */
        "MOV     %[tmp], #1               \n"  /* Value to store */
        "STREXB  %[res], %[tmp], [%[lock]] \n" /* Attempt store (res=0 success, res=1 fail) */
        "DMB                              \n"  /* Acquire barrier: Sync memory on success */
        "B       2f                       \n"  /* Exit */
        "1:                               \n"
        "CLREX                            \n" /* Clear exclusive monitor if STREXB was skipped */
        "MOV     %[res], #1               \n" /* Mark as failure */
        "2:                               \n"
        : [res] "=&r"(result), [tmp] "=&r"(tmp)
        : [lock] "r"(lock)
        : "cc", "memory");

    return (result == 0) ? 1U : 0U; /* 1 = acquired, 0 = failed */
}

OsStatus osMutexAcquire(Mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t irqState = osIrqSave();

    if ((mutex->locked != 0U) &&
        (mutex->ownerTask == osCurrentTask))
    {
        osIrqRestore(irqState);

        return OS_ERROR_ALREADY_OWNED;
    }

    if (!_mutexTryLock(&mutex->locked))
    {
        if (!_mutexEnqueue(mutex, osCurrentTask))
        {
            osIrqRestore(irqState);

            return OS_ERROR_WAIT_QUEUE_FULL;
        }

        _tcbs[osCurrentTask].waitReason = TASK_WAIT_MUTEX;
        _tcbs[osCurrentTask].state = TASK_BLOCKED;
        osScheduler();
        osRequestContextSwitch();
        osIrqRestore(irqState);

        /*
         * Once this task resumes, the mutex should have been
         * transferred to it by osMutexRelease().
         */
        return OS_OK;
    }

    mutex->ownerTask = osCurrentTask;

    osIrqRestore(irqState);

    return OS_OK;
}

OsStatus osMutexRelease(Mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t irqState = osIrqSave();

    if ((mutex->locked == 0U) ||
        (mutex->ownerTask != osCurrentTask))
    {
        osIrqRestore(irqState);

        return OS_ERROR_NOT_OWNER;
    }

    uint32_t nextOwner = _mutexDequeue(mutex);

    if (nextOwner != MUTEX_NO_OWNER)
    {
        /*
         * Keep the mutex locked and transfer ownership
         * directly to the next waiting task.
         */
        mutex->ownerTask = nextOwner;
        _tcbs[nextOwner].waitReason = TASK_WAIT_NONE;
        _tcbs[nextOwner].state = TASK_READY;
        osScheduler();
        osRequestContextSwitch();
        osIrqRestore(irqState);

        return OS_OK;
    }

    /*
     * No task is waiting, so fully unlock the mutex.
     */
    mutex->ownerTask = MUTEX_NO_OWNER;

    __asm volatile("DMB" ::: "memory");

    mutex->locked = 0U;

    osScheduler();

    osRequestContextSwitch();

    osIrqRestore(irqState);

    return OS_OK;
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
