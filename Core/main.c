#include <stdint.h>
#include <stddef.h>
#include "systick.h"
#include "os_kernel.h"
#include "semaphore.h"
#include "queue.h"
#include "uart.h"
#include "mem.h"

#define RCC_BASE 0x40023800UL
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOD_BASE 0x40020C00UL
#define GPIOD_MODER (*(volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_ODR (*(volatile uint32_t *)(GPIOD_BASE + 0x14))

#define LED_GREEN (1U << 12)
#define LED_ORANGE (1U << 13)
#define LED_RED (1U << 14)
#define LED_BLUE (1U << 15)

static Sem_t dataSem;
Sem_t logSem;
static Queue_t q;

void taskReceiver(void *arg)
{
    while (1)
    {

        SemWait(&dataSem);
        /* Data arrived — process it */
        char info[50];
        snprintf(info, sizeof(info), " Got semaphore. consumer: toggling blue.");
        SemWait(&logSem);
        uart4_println(info);
        SemSignal(&logSem);

        GPIOD_ODR ^= LED_BLUE;
        osTaskDelay(1000);
    }
}

void taskProducer(void *arg)
{

    while (1)
    {
        char info[100];

        osTaskDelay(2000);
        snprintf(info, sizeof(info), " Producer: toggling green.");
        SemWait(&logSem);
        uart4_println(info);
        SemSignal(&logSem);

        GPIOD_ODR ^= LED_GREEN;
        SemSignal(&dataSem);

        info[0] = '\0';
        snprintf(info, sizeof(info), " Producer: semaphore released curval: %d", dataSem.current);
        SemWait(&logSem);
        uart4_println(info);
        SemSignal(&logSem);
    }
}

void idleTask(void *arg)
{
    while (1)
    {
        __asm volatile("WFI");
    }
}

void producer(void *arg)
{
    // uint8_t max = 4;
    // uint8_t i = 0;
    // while (1)
    // {
    //     GPIOD_ODR ^= LED_GREEN;

    //     osQueueSend(&q, &i);
    //     osTaskDelay(1000);

    //     i = (i + 1) % max;
    // }'
    while (1)
    {
        GPIOD_ODR ^= LED_GREEN;

        char info[100];
        snprintf(info, sizeof(info), " Producer: toggling green.");
        SemWait(&logSem);
        uart4_println(info);
        SemSignal(&logSem);

        osTaskDelay(1000);
    }
}

void consumer(void *arg)
{

    while (1)
    {
        GPIOD_ODR ^= LED_BLUE;

        char info[50];
        snprintf(info, sizeof(info), " consumer: toggling blue.");
        SemWait(&logSem);

        uart4_println(info);
        SemSignal(&logSem);

        osTaskDelay(3000);
    }
    // while (1)
    // {

    //     uint8_t item;
    //     osQueueReceive(&q, &item);

    //     if (item == 0)
    //         GPIOD_ODR ^= LED_BLUE;
    //     else if (item == 1)
    //         GPIOD_ODR ^= LED_ORANGE;
    //     else if (item == 2)
    //         GPIOD_ODR ^= LED_RED;

    //     osTaskDelay(5000);
    // }
}

int main(void)
{
    __asm volatile("CPSID I");

    RCC_AHB1ENR |= (1U << 3);

    GPIOD_MODER &= ~(0xFFU << 24);
    GPIOD_MODER |= (0x55U << 24);

    uart4_init();
    osKernelInit();
    // char info[50];

    osQueueInit(&q, sizeof(uint8_t));

    SemInit(&dataSem, 0, 1);
    SemInit(&logSem, 1, 1);

    osTaskCreate(taskProducer, NULL);
    osTaskCreate(taskReceiver, NULL);
    osTaskCreate(idleTask, NULL);

    SysTick_Init(1000);
    osKernelStart();

    // Safely format a string into the 'info' buffer

    return 0;
}
