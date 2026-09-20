#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "os_status.h"

#define MUTEX_QUEUE_SIZE 8U
#define MUTEX_NO_OWNER UINT32_MAX

typedef struct mutex
{
    /* data */
    volatile uint8_t locked;
    volatile uint32_t ownerTask;
    uint32_t waitQueue[MUTEX_QUEUE_SIZE];
    uint8_t waitHead;
    uint8_t waitTail;
    uint8_t waitCount;

} Mutex_t;

OsStatus osMutexInit(Mutex_t *mutex);
OsStatus osMutexAcquire(Mutex_t *mutex);
OsStatus osMutexRelease(Mutex_t *mutex);

#endif