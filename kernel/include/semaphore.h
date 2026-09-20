#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>

#include "os_status.h"

#define SEM_QUEUE_SIZE 8U

#define SEM_NO_OWNER UINT32_MAX

typedef struct
{
    volatile uint32_t current;

    uint32_t maxCount;

    uint32_t waitQueue[SEM_QUEUE_SIZE];

    uint8_t waitHead;

    uint8_t waitTail;

    uint8_t waitCount;

} Sem_t;

OsStatus osSemaphoreInit(
    Sem_t *sem,
    uint32_t initialCount,
    uint32_t maxCount);

OsStatus osSemaphoreWait(Sem_t *sem);

OsStatus osSemaphoreSignal(Sem_t *sem);

#endif