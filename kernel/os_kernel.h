#ifndef OS_KERNEL_H
#define OS_KERNEL_H

#include <stdint.h>

/* Maximum number of tasks (including idle task) */
#define OS_MAX_TASKS 3

/* Task stack size in words (1 word = 4 bytes) */
#define OS_STACK_SIZE 256

/* Task states */
typedef enum
{
    TASK_READY = 0,
    TASK_RUNNING = 1,
    TASK_BLOCKED = 2,
    TASK_SUSPENDED = 3
} TaskState;

/* Task Control Block */
typedef struct
{
    volatile uint32_t *stackPtr; /* Saved stack pointer — MUST be first member */
    TaskState state;

    uint32_t delayTicks;

} TCB_t;

/* Public API */
void osKernelInit(void);
void osTaskCreate(void (*taskFunc)(void *), void *arg);
void osKernelStart(void);
void osTaskDelay(uint32_t ticks);

#endif