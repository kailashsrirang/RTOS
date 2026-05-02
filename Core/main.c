#include <stdint.h>
#include "systick.h"
#include "os_kernel.h"
#include "mutex.h"
#include <stddef.h>

#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOD_BASE 0x40020C00UL
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

#define LED_GREEN (1U << 12)
#define LED_ORANGE (1U << 13)

static Mutex_t sharedMutex;
static volatile uint32_t sharedCounter = 0;

void taskGreen(void *arg)
{
    while (1)
    {
        osMutexAcquire(&sharedMutex);
        sharedCounter++;
        GPIOD_ODR ^= LED_GREEN;
        osMutexRelease(&sharedMutex);
        osTaskDelay(500);
    }
}

void taskOrange(void *arg)
{
    while (1)
    {
        osMutexAcquire(&sharedMutex);
        sharedCounter++;
        GPIOD_ODR ^= LED_ORANGE;
        osMutexRelease(&sharedMutex);
        osTaskDelay(250);
    }
}

void idleTask(void *arg)
{
    while (1)
    {
        __asm volatile("WFI");
    }
}

int main(void)
{
    __asm volatile("CPSID I");

    RCC_AHB1ENR |= (1U << 3);
    GPIOD_MODER &= ~(0xFFU << 24);
    GPIOD_MODER |= (0x55U << 24);

    osKernelInit();
    osMutexInit(&sharedMutex);

    osTaskCreate(taskGreen, NULL);
    osTaskCreate(taskOrange, NULL);
    osTaskCreate(idleTask, NULL);

    SysTick_Init(1000);
    osKernelStart();

    return 0;
}