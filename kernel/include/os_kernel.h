#ifndef OS_KERNEL_H
#define OS_KERNEL_H

#include <stdbool.h>
#include <stdint.h>

#include "os_status.h"

/* Maximum number of tasks, including the internal idle task. */
#define OS_MAX_TASKS 8U

/* Task stack size in 32-bit words. */
#define OS_STACK_SIZE 256U

_Static_assert(OS_MAX_TASKS >= 2U, "The RTOS requires an idle task and at least one application task");
_Static_assert(OS_MAX_TASKS <= UINT8_MAX, "OS_MAX_TASKS exceeds the range of the task counter");

/* Public API */
OsStatus osKernelInit(void);
OsStatus osTaskCreate(void (*taskFunc)(void *), void *arg);

/*
 *   Starts task execution.
 *
 * Return an error if startup fails.
 * Does not return on success.
 */
OsStatus osKernelStart(void);
OsStatus osTaskDelay(uint32_t ticks);
void osTaskExit(void) __attribute__((noreturn));
bool osKernelIsRunning(void);
OsStatus osTaskGetStackHighWaterMark(uint32_t taskIndex, uint32_t *peakUsedWords);

#endif