#include <stdint.h>
#include <stddef.h>
#include "systick.h"
#include "os_kernel.h"
#include "semaphore.h"
#include "queue.h"
#include "uart.h"
#include "mem.h"
#include "led.h"
#include "gpio.h"
#include "button.h"
#include "os_status.h"
#include "os_interrupt.h"

#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOD_BASE 0x40020C00UL
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

#define LED_GREEN (1U << 12)
#define LED_ORANGE (1U << 13)
#define LED_RED (1U << 14)
#define LED_BLUE (1U << 15)

static volatile uint32_t taskCounters[7] = {0U};

static const uint32_t taskIndices[7] = {
    0U,
    1U,
    2U,
    3U,
    4U,
    5U,
    6U};

static volatile OsStatus extraTaskStatus;

static void countingTask(void *arg)
{
    const uint32_t *taskIndex = arg;

    while (1)
    {
        taskCounters[*taskIndex]++;

        osTaskDelay(100U + (*taskIndex * 10U));
    }
}

int main(void)
{
    (void)osIrqSave();

    osKernelInit();

    for (uint8_t i = 0U; i < 7U; i++)
    {
        OsStatus status = osTaskCreate(
            countingTask,
            (void *)&taskIndices[i]);

        if (status != OS_OK)
        {
            while (1)
            {
            }
        }
    }

    extraTaskStatus = osTaskCreate(
        countingTask,
        (void *)&taskIndices[0]);

    SysTick_Init(1000U);

    osKernelStart();

    while (1)
    {
    }
}
