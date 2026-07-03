#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>

#define SEM_QUEUE_SIZE 8
#define SEM_NO_OWNER 0xFFFFFFFFUL

typedef struct
{
    volatile uint32_t current;
    uint32_t maxCount;
    uint32_t waitQueue[SEM_QUEUE_SIZE];
    uint8_t waitHead;
    uint8_t waitTail;
    uint8_t waitCount;
} Sem_t;

void SemInit(Sem_t *s, uint32_t initialCount, uint32_t maxCount);
uint8_t SemWait(Sem_t *s);
uint8_t SemSignal(Sem_t *s);

#endif