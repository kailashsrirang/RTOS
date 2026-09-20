#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "os_kernel.h"
#include "os_status.h"
#include "semaphore.h"
#include "systick.h"

#define CPU_CLOCK_HZ 16000000UL
#define OS_TICK_HZ 1000U
#define TEST_TIMEOUT_TICKS 1000U

static Sem_t s;

static volatile OsStatus consumerWaitStatus;
static volatile OsStatus producerSignalStatus;
static volatile bool consumerComplete = false;
static volatile bool producerComplete = false;
static volatile bool semaphoreTestResult = false;

static volatile OsStatus storedSignalStatus;
static volatile bool storedSignalTestResult = false;
static volatile OsStatus fullSignalStatus;
static volatile bool fullSignalTestResult = false;
static volatile OsStatus immediateWaitStatus;
static volatile bool immediateWaitTestResult = false;

static volatile bool consumerAboutToWait = false;
static volatile bool consumerWasQueued = false;
static volatile OsStatus producerDelayStatus = OS_OK;

static volatile bool handoffTestResult = false;

static void monitorTask(void *arg)
{
    (void)arg;

    uint32_t startTick = osGetTick();

    while ((!consumerComplete) ||
           (!producerComplete))
    {
        uint32_t elapsedTicks =
            osGetTick() - startTick;

        if (elapsedTicks >= TEST_TIMEOUT_TICKS)
        {
            /*
             * One of the tasks failed to complete, possibly
             * because an incorrect mutex operation blocked.
             */
            semaphoreTestResult = false;

            __asm volatile("BKPT #0");

            while (1)
            {
                __asm volatile("WFI");
            }
        }

        OsStatus delayStatus = osTaskDelay(1U);

        if (delayStatus != OS_OK)
        {
            semaphoreTestResult = false;

            __asm volatile("BKPT #0");

            while (1)
            {
                __asm volatile("WFI");
            }
        }
    }
    handoffTestResult =
        (producerDelayStatus == OS_OK) &&
        consumerWasQueued &&
        (consumerWaitStatus == OS_OK) &&
        (producerSignalStatus == OS_OK) &&
        (s.current == 0U) &&
        (s.waitCount == 0U);

    // phase 2
    storedSignalStatus = osSemaphoreSignal(&s);
    storedSignalTestResult =
        (storedSignalStatus == OS_OK) &&
        (s.current == 1U) &&
        (s.waitCount == 0U);
    fullSignalStatus = osSemaphoreSignal(&s);
    fullSignalTestResult =
        (fullSignalStatus == OS_ERROR_RESOURCE_FULL) &&
        (s.current == 1U) &&
        (s.waitCount == 0U);
    immediateWaitStatus = osSemaphoreWait(&s);
    immediateWaitTestResult =
        (immediateWaitStatus == OS_OK) &&
        (s.current == 0U) &&
        (s.waitCount == 0U);

    semaphoreTestResult =
        handoffTestResult &&
        storedSignalTestResult &&
        fullSignalTestResult &&
        immediateWaitTestResult &&
        consumerComplete &&
        producerComplete;

    /*
     * Inspect mutextestResult, ownerTaskStatus,
     * secondTaskStatus, and m in GDB.
     */
    __asm volatile("BKPT #0");

    while (1)
    {
        __asm volatile("WFI");
    }
}

static void consumer(void *arg)

{
    (void)arg;
    consumerAboutToWait = true;

    consumerWaitStatus = osSemaphoreWait(&s);
    consumerAboutToWait = false;

    consumerComplete = true;
}

static void producer(void *arg)
{
    (void)arg;

    while (consumerAboutToWait)
    {
        producerDelayStatus = osTaskDelay(1U);

        if (producerDelayStatus != OS_OK)
        {
            producerComplete = true;
            return;
        }
    }

    /* Give the consumer time to enter the semaphore wait queue. */
    producerDelayStatus = osTaskDelay(1U);

    if (producerDelayStatus != OS_OK)
    {
        producerComplete = true;
        return;
    }

    consumerWasQueued = (s.waitCount == 1U);
    producerSignalStatus = osSemaphoreSignal(&s);

    producerComplete = true;
}

int main(void)
{

    OsStatus kernelStatus = osKernelInit();

    if (kernelStatus != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }

    if (osSemaphoreInit(&s, 0U, 1U) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }

    if (osTaskCreate(consumer, NULL) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    if (osTaskCreate(producer, NULL) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }

    if (osTaskCreate(monitorTask, NULL) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    OsStatus tickStatus = SysTick_Init(CPU_CLOCK_HZ, (uint32_t)OS_TICK_HZ);

    if (tickStatus != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }

    OsStatus startStatus = osKernelStart();

    // reachign here means kernel start failed
    if (startStatus != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
}
