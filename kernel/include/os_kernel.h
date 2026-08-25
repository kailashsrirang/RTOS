#ifndef OS_KERNEL_H
#define OS_KERNEL_H

#include <stdint.h>
#include <stddef.h>

#include "os_status.h"
#include "os_tcb_offsets.h"

/* Maximum number of tasks (including idle task). Slot 0 is for idle task */
#define OS_MAX_TASKS 8U

/* Task stack size in words (1 word = 4 bytes) */
#define OS_STACK_SIZE 256U

_Static_assert(
    OS_MAX_TASKS >= 2U,
    "The RTOS requires an idle task and at least one application task");

_Static_assert(
    OS_MAX_TASKS <= UINT8_MAX,
    "OS_MAX_TASKS exceeds the range of the task counter");

/* Task states */
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

/* Task Control Block */
typedef struct
{
    volatile uint32_t *stackPtr; /* Saved stack pointer — MUST be first member */
    TaskState state;
    uint32_t delayTicks;
    TaskWaitReason waitReason;
} TCB_t;

_Static_assert(offsetof(TCB_t, stackPtr) == OS_TCB_STACK_PTR_OFFSET_BYTES, "TCB stack pointer offset does not match context-switch assembly");
_Static_assert(sizeof(TCB_t) == OS_TCB_SIZE_BYTES, "TCB size does not match context-switch assembly");

/* Public API */
void osKernelInit(void);
OsStatus osTaskCreate(void (*taskFunc)(void *), void *arg);
void osKernelStart(void);
void osTaskDelay(uint32_t ticks);
void osTaskExit(void) __attribute__((noreturn));

#endif