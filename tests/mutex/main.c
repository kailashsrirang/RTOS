#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "os_kernel.h"
#include "os_status.h"
#include "mutex.h"
#include "systick.h"

#define CPU_CLOCK_HZ 16000000UL
#define OS_TICK_HZ 1000U
#define TEST_TIMEOUT_TICKS 1000U

static Mutex_t m;

static volatile OsStatus ownerTaskStatus;
static volatile bool ownerTaskComplete = false;
static volatile bool nonOwnerAttemptComplete = false;

static volatile bool secondTaskComplete = false;
static volatile OsStatus secondTaskStatus;

static volatile bool mutextestResult;
static void monitorTask(void *arg);
static void ownerTask(void *arg);
static void secondTask(void *arg);

static volatile bool ownerReady = false;

static void monitorTask(void *arg)
{
    (void)arg;

    uint32_t startTick = osGetTick();

    while ((!ownerTaskComplete) ||
           (!secondTaskComplete))
    {
        uint32_t elapsedTicks =
            osGetTick() - startTick;

        if (elapsedTicks >= TEST_TIMEOUT_TICKS)
        {
            /*
             * One of the tasks failed to complete, possibly
             * because an incorrect mutex operation blocked.
             */
            mutextestResult = false;

            __asm volatile("BKPT #0");

            while (1)
            {
                __asm volatile("WFI");
            }
        }

        OsStatus delayStatus = osTaskDelay(1U);

        if (delayStatus != OS_OK)
        {
            mutextestResult = false;

            __asm volatile("BKPT #0");

            while (1)
            {
                __asm volatile("WFI");
            }
        }
    }

    mutextestResult =
        (ownerTaskStatus == OS_ERROR_ALREADY_OWNED) &&
        (secondTaskStatus == OS_ERROR_NOT_OWNER) &&
        (m.locked == 0U) &&
        (m.ownerTask == MUTEX_NO_OWNER) &&
        (m.waitCount == 0U);

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

void ownerTask(void *arg)

{
    (void)arg;
    if (osMutexAcquire(&m) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    ownerReady = true;
    ownerTaskStatus = osMutexAcquire(&m);

    while (!secondTaskComplete)
    {
        osTaskDelay(1U); // wait
    }

    if (osMutexRelease(&m) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    ownerTaskComplete = true;
}

void secondTask(void *arg)
{
    (void)arg;

    while (!ownerReady)
    {
    }
    secondTaskStatus = osMutexRelease(&m);
    nonOwnerAttemptComplete = true;
    secondTaskComplete = true;
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

    if (osMutexInit(&m) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }

    if (osTaskCreate(&ownerTask, NULL) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    if (osTaskCreate(&secondTask, NULL) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }

    if (osTaskCreate(&monitorTask, NULL) != OS_OK)
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
