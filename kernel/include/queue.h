#ifndef OS_QUEUE_H
#define OS_QUEUE_H

#include <stdint.h>
#include "os_status.h"

#define QUEUE_MAX_ITEMS 16U
#define QUEUE_ITEM_SIZE 4U
#define QUEUE_NO_TASK UINT32_MAX
#define QUEUE_WAIT_SIZE 8U

typedef struct
{
    uint8_t buf[QUEUE_MAX_ITEMS][QUEUE_ITEM_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint8_t itemSize;
    uint32_t sendWait[QUEUE_WAIT_SIZE];
    uint8_t sendHead;
    uint8_t sendTail;
    uint8_t sendCount;
    uint32_t recvWait[QUEUE_WAIT_SIZE];
    uint8_t recvHead;
    uint8_t recvTail;
    uint8_t recvCount;
} Queue_t;

OsStatus osQueueInit(Queue_t *queue, uint8_t itemSize);
OsStatus osQueueSend(Queue_t *queue, const void *item);
OsStatus osQueueReceive(Queue_t *queue, void *item);

#endif