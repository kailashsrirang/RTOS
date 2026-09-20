#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "os_kernel.h"
#include "os_status.h"
#include "queue.h"
#include "systick.h"

#define CPU_CLOCK_HZ 16000000UL
#define OS_TICK_HZ 1000U

#define FIRST_DRAIN_COUNT (QUEUE_MAX_ITEMS / 2U)
#define TEST_VALUE_BASE UINT32_C(0x1000)
#define EXPECTED_OPERATION_COUNT (2U * (QUEUE_MAX_ITEMS + FIRST_DRAIN_COUNT))

typedef enum
{
    QUEUE_STAGE_NONE = 0,
    QUEUE_STAGE_INITIAL_FILL,
    QUEUE_STAGE_FIRST_DRAIN,
    QUEUE_STAGE_REFILL,
    QUEUE_STAGE_FINAL_DRAIN,
    QUEUE_STAGE_FINAL_INVARIANT
} QueueTestStage;

static Queue_t queue;

static volatile bool queueTestComplete = false;
static volatile bool queueTestResult = false;
static volatile OsStatus operationStatus = OS_OK;
static volatile QueueTestStage failureStage = QUEUE_STAGE_NONE;
static volatile uint32_t failureIndex = UINT32_MAX;
static volatile uint32_t failureExpectedValue = 0U;
static volatile uint32_t failureObservedValue = 0U;
static volatile uint32_t successfulOperations = 0U;

static void haltWithBreakpoint(void)
{
    __asm volatile("BKPT #0");

    while (1)
    {
        __asm volatile("WFI");
    }
}
static void queueTestTask(void *arg)
{

    (void)arg;
    bool passed = true;
    // send 16 items in queue
    for (uint32_t index = 0U; index < QUEUE_MAX_ITEMS; index++)
    {
        uint32_t value = TEST_VALUE_BASE + index;
        operationStatus = osQueueSend(&queue, &value);
        if (operationStatus != OS_OK)
        {
            passed = false;
            failureStage = QUEUE_STAGE_INITIAL_FILL;
            failureIndex = index;
            goto test_complete;
        }

        successfulOperations++;
    }

    // receive 8
    for (uint32_t index = 0U; index < FIRST_DRAIN_COUNT; index++)
    {
        uint32_t observedValue = 0U;
        uint32_t expectedValue = TEST_VALUE_BASE + index;

        operationStatus = osQueueReceive(&queue, &observedValue);

        if ((operationStatus != OS_OK) || (observedValue != expectedValue))
        {
            passed = false;
            failureStage = QUEUE_STAGE_FIRST_DRAIN;
            failureIndex = index;
            failureExpectedValue = expectedValue;
            failureObservedValue = observedValue;
            goto test_complete;
        }

        successfulOperations++;
    }
    // send 8 items
    for (uint32_t index = 0U;
         index < FIRST_DRAIN_COUNT;
         index++)
    {
        uint32_t value =
            TEST_VALUE_BASE + QUEUE_MAX_ITEMS + index;

        operationStatus = osQueueSend(&queue, &value);

        if (operationStatus != OS_OK)
        {
            passed = false;
            failureStage = QUEUE_STAGE_REFILL;
            failureIndex = index;
            goto test_complete;
        }

        successfulOperations++;
    }

    // receive 16
    for (uint32_t index = 0U; index < QUEUE_MAX_ITEMS; index++)
    {
        uint32_t observedValue = 0U;
        uint32_t expectedValue = TEST_VALUE_BASE + FIRST_DRAIN_COUNT + index;

        operationStatus = osQueueReceive(&queue, &observedValue);

        if ((operationStatus != OS_OK) ||
            (observedValue != expectedValue))
        {
            passed = false;
            failureStage = QUEUE_STAGE_FINAL_DRAIN;
            failureIndex = index;
            failureExpectedValue = expectedValue;
            failureObservedValue = observedValue;
            goto test_complete;
        }

        successfulOperations++;
    }

    if ((queue.count != 0U) ||
        (queue.head != FIRST_DRAIN_COUNT) ||
        (queue.tail != FIRST_DRAIN_COUNT) ||
        (queue.sendCount != 0U) ||
        (queue.recvCount != 0U) ||
        (successfulOperations != EXPECTED_OPERATION_COUNT))
    {
        passed = false;
        failureStage = QUEUE_STAGE_FINAL_INVARIANT;
    }

test_complete:
    queueTestResult = passed;
    queueTestComplete = true;
    haltWithBreakpoint();
}

int main(void)
{
    OsStatus kernelStatus = osKernelInit();

    if (kernelStatus != OS_OK)
    {
        haltWithBreakpoint();
    }

    OsStatus queueStatus =
        osQueueInit(&queue, (uint8_t)sizeof(uint32_t));

    if (queueStatus != OS_OK)
    {
        haltWithBreakpoint();
    }

    OsStatus taskStatus =
        osTaskCreate(queueTestTask, NULL);

    if (taskStatus != OS_OK)
    {
        haltWithBreakpoint();
    }

    OsStatus tickStatus =
        SysTick_Init(CPU_CLOCK_HZ, OS_TICK_HZ);

    if (tickStatus != OS_OK)
    {
        haltWithBreakpoint();
    }

    OsStatus startStatus = osKernelStart();

    /*
     * A successful kernel start never returns.
     */
    (void)startStatus;
    haltWithBreakpoint();
}