#include "semaphore.h"
#include <stdint.h>
#include "os_kernel.h"
#include "mem.h"
#include "os_interrupt.h"
#include "os_kernel_internal.h"

_Static_assert(SEM_QUEUE_SIZE > 0U, "Semaphore wait queue must contain at least one entry");
_Static_assert(SEM_QUEUE_SIZE <= UINT8_MAX, "Semaphore wait queue size exceeds the wait counters");
_Static_assert(SEM_QUEUE_SIZE >= (OS_MAX_TASKS - 1U), "Semaphore wait queue cannot hold every application task");

static uint8_t _semEnqueue(Sem_t *s, uint32_t taskIndex)
{
    if (s->waitCount >= SEM_QUEUE_SIZE)
    {
        return 0;
    }

    s->waitQueue[s->waitTail] = taskIndex;
    s->waitTail = (uint8_t)((s->waitTail + 1) % SEM_QUEUE_SIZE);
    s->waitCount++;

    return 1;
}
static uint32_t _semDequeue(Sem_t *s)
{
    if (s->waitCount == 0)
    {
        return SEM_NO_OWNER;
    }

    uint32_t newTaskIndex = s->waitQueue[s->waitHead];

    s->waitHead = (uint8_t)((s->waitHead + 1) % SEM_QUEUE_SIZE);
    s->waitCount--;

    return newTaskIndex;
}

OsStatus osSemaphoreInit(Sem_t *sem, uint32_t initialCount, uint32_t maxCount)
{
    if (sem == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (maxCount == 0U)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (initialCount > maxCount)
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

    sem->current = initialCount;
    sem->maxCount = maxCount;

    memset(sem->waitQueue, 0, sizeof(sem->waitQueue));
    sem->waitHead = 0U;
    sem->waitTail = 0U;
    sem->waitCount = 0U;

    return OS_OK;
}

OsStatus osSemaphoreWait(Sem_t *sem)
{
    if (sem == NULL)
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

    if (sem->current > 0U)
    {
        sem->current--;
        osIrqRestore(irqState);
        return OS_OK;
    }

    if (!_semEnqueue(sem, osCurrentTask))
    {
        osIrqRestore(irqState);
        return OS_ERROR_WAIT_QUEUE_FULL;
    }
    _tcbs[osCurrentTask].waitReason = TASK_WAIT_SEMAPHORE;
    _tcbs[osCurrentTask].state = TASK_BLOCKED;
    osScheduler();
    osRequestContextSwitch();
    osIrqRestore(irqState);

    /*
     * Execution resumes here after osSemaphoreSignal() wakes
     * this task and directly gives it the resource.
     */
    return OS_OK;
}

OsStatus osSemaphoreSignal(Sem_t *sem)
{
    if (sem == NULL)
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

    if (sem->waitCount > 0U)
    {
        uint32_t nextTask = _semDequeue(sem);
        if (nextTask != SEM_NO_OWNER)
        {
            /*
             * Transfer the resource directly to the
             * waiting task instead of incrementing current.
             */
            _tcbs[nextTask].waitReason = TASK_WAIT_NONE;
            _tcbs[nextTask].state = TASK_READY;
            osScheduler();
            osRequestContextSwitch();
            osIrqRestore(irqState);
            return OS_OK;
        }
    }

    if (sem->current >= sem->maxCount)
    {
        osIrqRestore(irqState);
        return OS_ERROR_RESOURCE_FULL;
    }

    sem->current++;
    osIrqRestore(irqState);
    return OS_OK;
}
