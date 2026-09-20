#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "os_kernel.h"
#include "os_status.h"
#include "queue.h"
#include "systick.h"

#define CPU_CLOCK_HZ 16000000UL
#define OS_TICK_HZ 1000U
#define TEST_ITEM_COUNT 3U

static Queue_t queue;

static const uint32_t sendItems[TEST_ITEM_COUNT] = {0x11111111UL, 0x22222222UL, 0x33333333UL};
static volatile uint32_t receiveItems[TEST_ITEM_COUNT];

static volatile OsStatus sendStatus[TEST_ITEM_COUNT];
static volatile OsStatus receiveStatus[TEST_ITEM_COUNT];

static volatile bool itemsValid[TEST_ITEM_COUNT];

static volatile bool sendPassed = true;
static volatile bool receivePassed = true;

static volatile bool queueTestComplete = false;
static volatile bool queueTestResult = false;

static volatile uint32_t sendsCompleted = 0U;
static volatile uint32_t receivesCompleted = 0U;

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

    bool allItemsValid = true;

    for (uint32_t i = 0; i < TEST_ITEM_COUNT; i++)
    {

        OsStatus status = osQueueSend(&queue, &sendItems[i]);
        sendStatus[i] = status;

        if (status != OS_OK)
        {
            sendPassed = false;
            break;
        }
        sendsCompleted++;
    }
    // receive
    if (sendPassed)
    {
        for (uint32_t i = 0; i < TEST_ITEM_COUNT; i++)
        {

            uint32_t receivedItem = 0U;

            OsStatus status =
                osQueueReceive(&queue, &receivedItem);

            receiveStatus[i] = status;
            receiveItems[i] = receivedItem;

            if (status != OS_OK)
            {
                receivePassed = false;
                break;
            }
            receivesCompleted++;

            // validate values
            itemsValid[i] = (receivedItem == sendItems[i]);

            if (!itemsValid[i])
            {
                allItemsValid = false;
            }
        }
    }

    queueTestResult =
        sendPassed &&
        receivePassed &&
        allItemsValid;
    queueTestComplete = true;
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