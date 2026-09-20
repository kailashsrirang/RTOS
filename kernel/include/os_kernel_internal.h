#ifndef OS_KERNEL_INTERNAL_H
#define OS_KERNEL_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "os_kernel.h"
#include "os_tcb_offsets.h"

typedef enum
{
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} TaskState;

typedef enum
{
    TASK_WAIT_NONE = 0,
    TASK_WAIT_DELAY,
    TASK_WAIT_MUTEX,
    TASK_WAIT_SEMAPHORE,
    TASK_WAIT_QUEUE_SEND,
    TASK_WAIT_QUEUE_RECEIVE
} TaskWaitReason;

typedef struct
{
    volatile uint32_t *stackPtr;
    TaskState state;
    uint32_t delayTicks;
    TaskWaitReason waitReason;
} TCB_t;

_Static_assert(offsetof(TCB_t, stackPtr) == OS_TCB_STACK_PTR_OFFSET_BYTES, "TCB stack pointer offset does not match assembly");

_Static_assert(sizeof(TCB_t) == OS_TCB_SIZE_BYTES, "TCB size does not match assembly");

extern volatile TCB_t _tcbs[OS_MAX_TASKS];
extern volatile uint32_t osCurrentTask;
extern volatile uint32_t osNextTask;

void osScheduler(void);
void SysTick_Handler(void);

#endif /* OS_KERNEL_INTERNAL_H */