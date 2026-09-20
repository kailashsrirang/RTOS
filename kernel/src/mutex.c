#include "mutex.h"
#include <stdint.h>
#include <stddef.h>
#include "os_kernel.h"
#include "mem.h"
#include "os_interrupt.h"
#include "os_kernel_internal.h"

static bool _mutexTryLock(volatile uint8_t *locked);
static bool _mutexEnqueue(Mutex_t *mutex, uint32_t taskIndex);
static uint32_t _mutexDequeue(Mutex_t *mutex);

_Static_assert(MUTEX_QUEUE_SIZE > 0U, "Mutex wait queue must contain at least one entry");
_Static_assert(MUTEX_QUEUE_SIZE <= UINT8_MAX, "Mutex wait queue size exceeds the wait counters");
_Static_assert(MUTEX_QUEUE_SIZE >= (OS_MAX_TASKS - 1U), "Mutex wait queue cannot hold every application task");
OsStatus osMutexInit(Mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (osIsInInterruptContext())
    {
        return OS_ERROR_ISR_CONTEXT;
    }

    if (osKernelIsRunning())
    {
        return OS_ERROR_INVALID_STATE;
    }

    mutex->waitCount = 0U;

    mutex->waitHead = 0U;

    mutex->waitTail = 0U;

    memset(mutex->waitQueue, 0, sizeof(mutex->waitQueue));

    mutex->locked = 0U;

    mutex->ownerTask = MUTEX_NO_OWNER;

    return OS_OK;
}

/*
 * Protected check-and-set
 * The caller must hold the kernel crit section
 */
static bool _mutexTryLock(volatile uint8_t *lock)
{
    // caler will hold crit section
    if (*lock != 0U)
    {
        return false;
    }

    *lock = 1U;

    __asm volatile("DMB" ::: "memory");

    return true;
}

OsStatus osMutexAcquire(Mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (!osKernelIsRunning())
    {
        return OS_ERROR_INVALID_STATE;
    }

    if (osIsInInterruptContext())
    {
        return OS_ERROR_ISR_CONTEXT; // executing in handler mode. ISR can't amke blocking calls.
    }

    if (osAreInterruptsDisabled())
    {
        return OS_ERROR_INTERRUPTS_DISABLED; // interrupts already disabled in caller
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

    if (!osKernelIsRunning())
    {
        return OS_ERROR_INVALID_STATE;
    }

    if (osIsInInterruptContext())
    {
        return OS_ERROR_ISR_CONTEXT; // executing in handler mode. ISR can't amke blocking calls.
    }

    if (osAreInterruptsDisabled())
    {
        return OS_ERROR_INTERRUPTS_DISABLED; // interrupts already disabled in caller
    }

    uint32_t irqState = osIrqSave();

    if ((mutex->locked == 0U) ||
        (mutex->ownerTask != osCurrentTask))
    {
        osIrqRestore(irqState);

        return OS_ERROR_NOT_OWNER;
    }

    uint32_t nextOwner = _mutexDequeue(mutex);

    __asm volatile("DMB" ::: "memory");

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

    mutex->locked = 0U;

    osIrqRestore(irqState);

    return OS_OK;
}

static bool _mutexEnqueue(Mutex_t *mutex, uint32_t taskIndex)
{
    if (mutex->waitCount >= MUTEX_QUEUE_SIZE)
    {
        return 0;
    }

    mutex->waitQueue[mutex->waitTail] = taskIndex;
    mutex->waitTail = (uint8_t)((mutex->waitTail + 1U) % MUTEX_QUEUE_SIZE);
    mutex->waitCount++;

    return 1;
}
static uint32_t _mutexDequeue(Mutex_t *mutex)
{
    if (mutex->waitCount == 0)
    {
        return MUTEX_NO_OWNER;
    }

    uint32_t newTaskIndex = mutex->waitQueue[mutex->waitHead];

    mutex->waitHead = (uint8_t)((mutex->waitHead + 1) % MUTEX_QUEUE_SIZE);
    mutex->waitCount--;

    return newTaskIndex;
}
