#include "os_kernel.h"
#include "mem.h"
#include <stddef.h>
#include <stdint.h>
#include "os_interrupt.h"

#define SCB_SHPR3 (*(volatile uint32_t *)0xE000ED20UL)

#define SCB_SHPR3_PENDSV_SHIFT 16U
#define SCB_SHPR3_SYSTICK_SHIFT 24U

#define OS_PENDSV_PRIORITY 0xF0U
#define OS_SYSTICK_PRIORITY 0xE0U

#define OS_INVALID_TASK_INDEX UINT32_MAX

static uint32_t _taskStacks[OS_MAX_TASKS][OS_STACK_SIZE]
    __attribute__((aligned(8))); /* Static stack storage for all tasks */

volatile TCB_t _tcbs[OS_MAX_TASKS]; /* TCB array — one per task slot */

static volatile uint8_t _taskCount = 0; /* Number of tasks created so far */
extern volatile uint32_t _osTick;
volatile uint32_t osCurrentTask = 0; /* Index of currently running task */
volatile uint32_t osNextTask = 0;    /* Index of next task to run (set by scheduler) */
static uint32_t _idleTaskIndex = OS_INVALID_TASK_INDEX;

static void osIdleTask(void *arg)
{
    (void)arg;

    while (1)
    {
        /*
         * Sleep until an interrupt occurs.
         *
         * SysTick will wake the processor every tick and check
         * whether any application task has become ready.
         */
        __asm volatile("WFI");
    }
}

static void _initTaskStack(uint8_t taskIndex, void (*taskFunc)(void *), void *arg)
{
    uint32_t *stack = _taskStacks[taskIndex];
    uint32_t stackTop = OS_STACK_SIZE;

    /* Simulate the hardware exception stack frame */
    /* Fill in reverse — we're placing items at top of stack going downward */

    stack[--stackTop] = (1U << 24);                  /* xPSR: Thumb bit must be set */
    stack[--stackTop] = ((uint32_t)taskFunc) | 1U;   /* PC: task entry point */
    stack[--stackTop] = ((uint32_t)osTaskExit) | 1U; /* LR: task return address */
    stack[--stackTop] = 0x12121212UL;                /* R12: dummy value for debugging */
    stack[--stackTop] = 0x03030303UL;                /* R3 */
    stack[--stackTop] = 0x02020202UL;                /* R2 */
    stack[--stackTop] = 0x01010101UL;                /* R1 */
    stack[--stackTop] = (uint32_t)arg;               /* R0: first argument to task function */

    /* Manually saved registers R4-R11 */
    stack[--stackTop] = 0x11111111UL; /* R11 */
    stack[--stackTop] = 0x10101010UL; /* R10 */
    stack[--stackTop] = 0x09090909UL; /* R9 */
    stack[--stackTop] = 0x08080808UL; /* R8 */
    stack[--stackTop] = 0x07070707UL; /* R7 */
    stack[--stackTop] = 0x06060606UL; /* R6 */
    stack[--stackTop] = 0x05050505UL; /* R5 */
    stack[--stackTop] = 0x04040404UL; /* R4 */

    _tcbs[taskIndex].stackPtr = &stack[stackTop];
}

static void osConfigureExceptionPriorities(void)
{
    uint32_t registerValue = SCB_SHPR3;

    registerValue &= ~((UINT32_C(0xFF) << SCB_SHPR3_PENDSV_SHIFT) | (UINT32_C(0xFF) << SCB_SHPR3_SYSTICK_SHIFT));

    registerValue |= ((uint32_t)OS_PENDSV_PRIORITY << SCB_SHPR3_PENDSV_SHIFT);
    registerValue |= ((uint32_t)OS_SYSTICK_PRIORITY << SCB_SHPR3_SYSTICK_SHIFT);
    SCB_SHPR3 = registerValue;
}

void osKernelInit(void)
{
    /* Clear any pending SysTick or PendSV exceptions */
    osClearPendingSchedulerExceptions();

    /*
     * SysTick runs before PendSV and PendSV is lowest-priority kernel exception.
     */
    osConfigureExceptionPriorities();

    _taskCount = 0;
    osCurrentTask = 0;
    osNextTask = 0;
    memset(_tcbs, 0, sizeof(_tcbs));
    memset(_taskStacks, 0, sizeof(_taskStacks));

    _idleTaskIndex = _taskCount; /* First task is the idle task */
    if (osTaskCreate(osIdleTask, NULL) != OS_OK)
    {
        while (1)
        {
            __asm volatile("BKPT #0");
        }
    }
    _tcbs[_idleTaskIndex].state = TASK_RUNNING;
}

OsStatus osTaskCreate(void (*taskFunc)(void *), void *arg)
{
    if (taskFunc == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (_taskCount >= OS_MAX_TASKS)
    {
        return OS_ERROR_NO_TASK_SLOTS;
    }

    uint8_t taskIndex = _taskCount;
    _tcbs[taskIndex].state = TASK_READY;
    _tcbs[taskIndex].waitReason = TASK_WAIT_NONE;
    _tcbs[taskIndex].delayTicks = 0U;
    _initTaskStack(taskIndex, taskFunc, arg);
    _taskCount++;

    return OS_OK;
}
void osScheduler(void)
{
    if (_taskCount == 0U)
    {
        return;
    }

    uint32_t selectedTask = _idleTaskIndex;
    uint8_t next = (uint8_t)((osCurrentTask + 1U) % _taskCount);

    /*
     * First look for a runnable non-idle task.
     */
    for (uint8_t i = 0U; i < _taskCount; i++)
    {
        if ((next != _idleTaskIndex) &&
            ((_tcbs[next].state == TASK_READY) ||
             (_tcbs[next].state == TASK_RUNNING)))
        {
            selectedTask = next;
            break;
        }

        next = (uint8_t)((next + 1U) % _taskCount);
    }
    if ((selectedTask != osCurrentTask) &&
        (_tcbs[osCurrentTask].state == TASK_RUNNING))
    {
        _tcbs[osCurrentTask].state = TASK_READY;
    }

    _tcbs[selectedTask].state = TASK_RUNNING;

    osNextTask = selectedTask;
    // osNextTask = _idleTaskIndex; /* No application task is runnable so run idle task */
}

void SysTick_Handler(void)
{
    _osTick++;

    for (uint8_t i = 0; i < _taskCount; i++)
    {
        if (_tcbs[i].state == TASK_BLOCKED && _tcbs[i].waitReason == TASK_WAIT_DELAY)
        {
            if (_tcbs[i].delayTicks > 0)
            {
                _tcbs[i].delayTicks--;
                if (_tcbs[i].delayTicks == 0)
                {
                    _tcbs[i].waitReason = TASK_WAIT_NONE;
                    _tcbs[i].state = TASK_READY;
                }
            }
        }
    }

    osScheduler();

    if (osNextTask != osCurrentTask) /* Prevent useless context switch */
    {
        // char info[50];
        // snprintf(info, sizeof(info), "Os scheduler chose osNextTask %d. PendingSV...", osNextTask);
        // uart4_println(info);

        osRequestContextSwitch();
    }
}

void osTaskDelay(uint32_t ticks)
{
    if (ticks == 0U)
    {
        return;
    }

    uint32_t irqState = osIrqSave();

    _tcbs[osCurrentTask].delayTicks = ticks;
    _tcbs[osCurrentTask].state = TASK_BLOCKED;
    _tcbs[osCurrentTask].waitReason = TASK_WAIT_DELAY;

    osScheduler();

    osRequestContextSwitch(); /* PendSV set pending */

    osIrqRestore(irqState); /* Return interrupt Enable/Disable back to original state */
}

__attribute__((naked, noreturn)) void osKernelStart(void)
{
    /* Point PSP at task 0's saved stack */
    __asm volatile(
        "LDR R0, =_tcbs             \n" /* R0 = base of TCB array */
        "LDR R1, [R0]               \n" /* R1 = _tcbs[0].stackPtr */
        "MSR PSP, R1                \n" /* PSP = task 0 stack pointer */
        "MOV R0, #0x02              \n"
        "MSR CONTROL, R0            \n" /* Switch to PSP in thread mode */
        "ISB                        \n"
        "POP {R4-R11}               \n" /* Restore R4-R11 from fake frame */
        "POP {R0-R3, R12, LR}      \n"  /* Restore R0-R3, R12, LR */

        "POP {R1}                  \n" /* Restore PC register */
        "ADD SP, SP, #4            \n" /* Skip xPSR register */
        "ORR R1, R1, #1            \n"
        "CPSIE I                    \n" /* Enable interrupts */
        "BX R1                     \n"
        // "POP {PC}                   \n" /* Jump to task 0 entry point */

    );
}

void osTaskExit(void)
{
    uint32_t irqState = osIrqSave();

    _tcbs[osCurrentTask].delayTicks = 0U;

    _tcbs[osCurrentTask].waitReason = TASK_WAIT_NONE;

    _tcbs[osCurrentTask].state = TASK_TERMINATED;

    osScheduler();

    osRequestContextSwitch();

    osIrqRestore(irqState);

    // task cant return bc there is no valid caller. therefore must be in inifnite loop
    while (1)
    {
        __asm volatile("WFI");
    }
}
