#include "queue.h"
#include <stdint.h>
#include "os_kernel.h"
#include <string.h>

#define ICSR (*(volatile uint32_t *)0xE000ED04UL)

extern volatile uint32_t osCurrentTask;
extern volatile TCB_t _tcbs[OS_MAX_TASKS];

static uint32_t _dequeue(uint32_t *q, uint8_t *head, uint8_t *count)
{
    if (*count == 0)
        return QUEUE_NO_TASK;
    uint32_t nextTask = q[*head];
    *head = (*head + 1) % QUEUE_WAIT_SIZE;
    (*count)--;
    return nextTask;
}

static void _enqueue(uint32_t *q, uint8_t *tail, uint8_t *count, uint32_t value)
{
    if (*count >= QUEUE_WAIT_SIZE)
    {
        /* queue full */
        return;
    }
    q[*tail] = value;
    *tail = (*tail + 1) % QUEUE_WAIT_SIZE;
    (*count)++;
}

static void _copyIn(Queue_t *q, void *item)
{
    memcpy(q->buf[q->tail], item, q->itemSize);
    q->count++;
    q->tail = (q->tail + 1) % QUEUE_MAX_ITEMS;
}

static void _copyOut(Queue_t *q, void *item)
{
    memcpy(item, q->buf[q->head], q->itemSize);
    q->count--;
    q->head = (q->head + 1) % QUEUE_MAX_ITEMS;
}

void osQueueInit(Queue_t *q, uint8_t itemSize)
{
    memset(q, 0, sizeof(Queue_t));
    q->itemSize = itemSize;
}

void osQueueSend(Queue_t *q, void *item)
{
    __asm volatile("CPSID I");

    while (q->count >= QUEUE_MAX_ITEMS)
    {
        _enqueue(q->sendWait, &(q->sendTail), &(q->sendCount), osCurrentTask);

        _tcbs[osCurrentTask].state = TASK_BLOCKED;

        ICSR |= (1U << 28); // PendSV set pending

        __asm volatile("CPSIE I");

        // Task should context switch away here.
        // When it later resumes, re-enter critical section and re-check.
        __asm volatile("CPSID I");
    }

    _copyIn(q, item);

    if (q->recvCount > 0)
    {
        uint32_t nextTask = _dequeue(q->recvWait, &(q->recvHead), &(q->recvCount));

        if (nextTask != QUEUE_NO_TASK)
        {
            _tcbs[nextTask].state = TASK_READY;
        }

        ICSR |= (1U << 28);
    }

    __asm volatile("CPSIE I");
}

void osQueueReceive(Queue_t *q, void *item)
{
    __asm volatile("CPSID I");

    while (q->count == 0)
    {
        _enqueue(q->recvWait, &(q->recvTail), &(q->recvCount), osCurrentTask);

        _tcbs[osCurrentTask].state = TASK_BLOCKED;

        ICSR |= (1U << 28); // PendSV set pending

        __asm volatile("CPSIE I");

        // Task should context switch away here.
        // When it resumes, re-enter critical section and re-check.
        __asm volatile("CPSID I");
    }

    _copyOut(q, item);

    if (q->sendCount > 0)
    {
        uint32_t nextTask = _dequeue(q->sendWait, &(q->sendHead), &(q->sendCount));

        if (nextTask != QUEUE_NO_TASK)
        {
            _tcbs[nextTask].state = TASK_READY;
        }

        ICSR |= (1U << 28);
    }

    __asm volatile("CPSIE I");
}
