#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "os_kernel.h"
#include "os_status.h"
#include "mutex.h"
#include "systick.h"

#define CPU_CLOCK_HZ 16000000UL
#define OS_TICK_HZ 1000U

static Mutex_t m;
static volatile uint32_t sharedVal = 0;
static volatile bool mutextestResult;
static volatile bool w0completed = false;
static volatile bool w1completed = false;
static void monitorTask(void *arg);
static void workerTask(void *arg);

uint8_t workerIds[] = {
    0U, 1U};

void monitorTask(void *arg)
{
    (void)arg;
    while ((!w0completed) || (!w1completed))
    {
        if (osTaskDelay(1U) != OS_OK)
        {
            while (1)
            {
                __asm volatile("BKPT #0");
            }
        }
    }
    if (sharedVal != 200)
    {
        mutextestResult = false;
    }
    else
    {
        mutextestResult = true;
    }
    while (1)
    {
        __asm volatile("BKPT #0");
    }
}
void workerTask(void *arg)
{
    uint8_t id = *(uint8_t *)arg;
    for (int i = 0; i < 100; i++)
    {

        if (osMutexAcquire(&m) != OS_OK)
        {
            while (1)
            {
                __asm volatile("BKPT #0");
            }
        }
        uint32_t localvar = sharedVal;
        if (osTaskDelay(1U) != OS_OK)
        {
            while (1)
            {
                __asm volatile("BKPT #0");
            }
        }
        localvar++;
        sharedVal = localvar;
        if (osMutexRelease(&m) != OS_OK)
        {
            while (1)
            {
                __asm volatile("BKPT #0");
            }
        }
    }
    if (id == 0)
    {
        w0completed = true;
    }
    else
    {
        w1completed = true;
    }
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

    if (osTaskCreate(&workerTask, &workerIds[0]) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    if (osTaskCreate(&workerTask, &workerIds[1]) != OS_OK)
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
