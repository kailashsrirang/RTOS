#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>

typedef struct mutex
{
    /* data */
    volatile uint8_t locked;
    volatile uint8_t ownerTask;

} Mutex_t;

void osMutexInit(Mutex_t *mutex);
void osMutexAcquire(Mutex_t *mutex);
void osMutexRelease(Mutex_t *mutex);

#endif