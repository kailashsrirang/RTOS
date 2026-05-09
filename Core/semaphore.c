#include "semaphore.h"
#include <stdint.h>
#include "os_kernel.h"
#include <string.h>

#define ICSR (*(volatile uint32_t *)0xE000ED04UL)

extern volatile uint32_t osCurrentTask;
extern volatile TCB_t _tcbs[OS_MAX_TASKS];

static uint8_t _semEnqueue(Sem_t *s, uint32_t taskIndex)
{
    if (s->waitCount >= SEM_QUEUE_SIZE)
    {
        return 0;
    }

    s->waitQueue[s->waitTail] = taskIndex;
    s->waitTail = (s->waitTail + 1) % SEM_QUEUE_SIZE;
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

    s->waitHead = (s->waitHead + 1) % SEM_QUEUE_SIZE;
    s->waitCount--;

    return newTaskIndex;
}

void SemInit(Sem_t *s, uint32_t initialCount, uint32_t maxCount)
{
    if (initialCount > maxCount)
    {
        initialCount = maxCount;
    }
    s->current = initialCount;
    s->maxCount = maxCount;
    memset(&s->waitQueue, 0, sizeof(s->waitQueue));
    s->waitHead = 0;
    s->waitTail = 0;
    s->waitCount = 0;
}

uint8_t SemWait(Sem_t *s)
{
    __asm volatile("CPSID I");

    if (s->current > 0)
    {
        /* have resources availble */
        s->current--;
        __asm volatile("CPSIE I");
        return 1;
    }
    else
    {
        /* need to wait*/
        if (!_semEnqueue(s, osCurrentTask))
        {
            __asm volatile("CPSIE I");
            return 0;
        }
        _tcbs[osCurrentTask].state = TASK_BLOCKED;
        ICSR = (1U << 28); /* PendSV set pending */
        __asm volatile("CPSIE I");
        return 1;
    }
}

uint8_t SemSignal(Sem_t *s)
{
    __asm volatile("CPSID I");

    if (s->waitCount > 0)
    {
        uint32_t nextTask = _semDequeue(s);

        if (nextTask != SEM_NO_OWNER)
        {

            _tcbs[nextTask].state = TASK_READY;

            ICSR = (1U << 28); /* PendSV set pending */
            __asm volatile("CPSIE I");
            return 1;
        }
    }
    if (s->current < s->maxCount)
    {

        s->current++;

        __asm volatile("CPSIE I");
        return 1;
    }

    /* Semaphore already full */
    __asm volatile("CPSIE I");
    return 0;
}
