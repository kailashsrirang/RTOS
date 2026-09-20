#include "queue.h"
#include <stdint.h>
#include "os_kernel.h"
#include "mem.h"
#include "os_interrupt.h"
#include "os_kernel_internal.h"

_Static_assert(QUEUE_WAIT_SIZE >= (OS_MAX_TASKS - 1U), "Queue waiting list cannot hold every application task");
_Static_assert(QUEUE_WAIT_SIZE > 0U, "Queue wait list must contain at least one entry");
_Static_assert(QUEUE_WAIT_SIZE <= UINT8_MAX, "Queue wait list size exceeds the wait counters");
_Static_assert(QUEUE_MAX_ITEMS > 0U, "Queue must hold at least one item");
_Static_assert(QUEUE_MAX_ITEMS <= UINT8_MAX, "Queue capacity exceeds the item counters");
_Static_assert(QUEUE_ITEM_SIZE > 0U, "Queue items must contain at least one byte");
_Static_assert(QUEUE_ITEM_SIZE <= UINT8_MAX, "Queue item size exceeds the itemSize field");

static uint32_t _dequeue(uint32_t *queue, uint8_t *head, uint8_t *count)
{
    if (*count == 0)
        return QUEUE_NO_TASK;
    uint32_t nextTask = queue[*head];
    *head = (uint8_t)((*head + 1) % QUEUE_WAIT_SIZE);
    (*count)--;
    return nextTask;
}

static OsStatus _enqueue(uint32_t *queue, uint8_t *tail, uint8_t *count, uint32_t taskIndex)
{
    if (*count >= QUEUE_WAIT_SIZE)
    {
        /* queue full */
        return OS_ERROR_WAIT_QUEUE_FULL;
    }
    queue[*tail] = taskIndex;
    *tail = (uint8_t)((*tail + 1) % QUEUE_WAIT_SIZE);
    (*count)++;
    return OS_OK;
}

static void _copyIn(Queue_t *queue, const void *item)
{
    memcpy(queue->buf[queue->tail], item, queue->itemSize);
    queue->count++;
    queue->tail = (uint8_t)((queue->tail + 1U) % QUEUE_MAX_ITEMS);
}

static void _copyOut(Queue_t *queue, void *item)
{
    memcpy(item, queue->buf[queue->head], queue->itemSize);
    queue->count--;
    queue->head = (uint8_t)((queue->head + 1U) % QUEUE_MAX_ITEMS);
}

OsStatus osQueueInit(Queue_t *queue, uint8_t itemSize)
{
    if (queue == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (itemSize == 0U)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (itemSize > QUEUE_ITEM_SIZE)
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

    memset(queue, 0, sizeof(*queue));
    queue->itemSize = itemSize;

    return OS_OK;
}

OsStatus osQueueSend(Queue_t *queue, const void *item)
{
    if ((queue == NULL) || (item == NULL))
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

    while (queue->count >= QUEUE_MAX_ITEMS)
    {
        OsStatus enqueueStatus = _enqueue(queue->sendWait, &queue->sendTail, &queue->sendCount, osCurrentTask);
        if (enqueueStatus != OS_OK)
        {
            osIrqRestore(irqState);
            return enqueueStatus;
        }
        _tcbs[osCurrentTask].waitReason = TASK_WAIT_QUEUE_SEND;
        _tcbs[osCurrentTask].state = TASK_BLOCKED;
        osScheduler();
        osRequestContextSwitch();

        /*Let PendSV switch this task out.*/
        osIrqRestore(irqState);

        /*
         * When the task resumes, protect the queue
         * before checking its state again.
         */
        irqState = osIrqSave();
    }

    _copyIn(queue, item);
    if (queue->recvCount > 0U)
    {
        uint32_t nextTask = _dequeue(queue->recvWait, &queue->recvHead, &queue->recvCount);

        if (nextTask != QUEUE_NO_TASK)
        {
            _tcbs[nextTask].waitReason = TASK_WAIT_NONE;
            _tcbs[nextTask].state = TASK_READY;
            osScheduler();
            osRequestContextSwitch();
        }
    }

    osIrqRestore(irqState);

    return OS_OK;
}
OsStatus osQueueReceive(Queue_t *queue, void *item)
{
    if ((queue == NULL) || (item == NULL))
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

    while (queue->count == 0U)
    {
        OsStatus enqueueStatus = _enqueue(queue->recvWait, &queue->recvTail, &queue->recvCount, osCurrentTask);

        if (enqueueStatus != OS_OK)
        {
            osIrqRestore(irqState);
            return enqueueStatus;
        }
        _tcbs[osCurrentTask].waitReason = TASK_WAIT_QUEUE_RECEIVE;
        _tcbs[osCurrentTask].state = TASK_BLOCKED;
        osScheduler();
        osRequestContextSwitch();

        /*
         * Allow the task to be switched out while
         * waiting for a message.
         */
        osIrqRestore(irqState);

        /*
         * Once awakened, protect the queue before
         * checking whether a message is available.
         */
        irqState = osIrqSave();
    }

    _copyOut(queue, item);

    if (queue->sendCount > 0U)
    {
        uint32_t nextTask = _dequeue(queue->sendWait, &queue->sendHead, &queue->sendCount);
        if (nextTask != QUEUE_NO_TASK)
        {
            _tcbs[nextTask].waitReason = TASK_WAIT_NONE;
            _tcbs[nextTask].state = TASK_READY;
            osScheduler();
            osRequestContextSwitch();
        }
    }

    osIrqRestore(irqState);

    return OS_OK;
}
