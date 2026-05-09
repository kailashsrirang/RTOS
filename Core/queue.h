#ifndef OS_QUEUE_H
#define OS_QUEUE_H

#include <stdint.h>

#define QUEUE_MAX_ITEMS 16
#define QUEUE_ITEM_SIZE 4
#define QUEUE_NO_TASK 0xFFFFFFFFUL
#define QUEUE_WAIT_SIZE 8

typedef struct
{
    uint8_t buf[QUEUE_MAX_ITEMS][QUEUE_ITEM_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint8_t itemSize;

    /* Tasks blocked waiting to send (queue full) */
    uint32_t sendWait[QUEUE_WAIT_SIZE];
    uint8_t sendHead;
    uint8_t sendTail;
    uint8_t sendCount;

    /* Tasks blocked waiting to receive (queue empty) */
    uint32_t recvWait[QUEUE_WAIT_SIZE];
    uint8_t recvHead;
    uint8_t recvTail;
    uint8_t recvCount;
} Queue_t;

void osQueueInit(Queue_t *q, uint8_t itemSize);
void osQueueSend(Queue_t *q, void *item);
void osQueueReceive(Queue_t *q, void *item);
void osQueueSendFromISR(Queue_t *q, void *item);

#endif