#include "os_kernel.h"
#include "mem.h"
#include <stddef.h>
#include <stdint.h>
#include "os_interrupt.h"
#include "os_kernel_internal.h"

#define SCB_SHPR3 (*(volatile uint32_t *)0xE000ED20UL)
#define SCB_SHPR3_PENDSV_SHIFT 16U
#define SCB_SHPR3_SYSTICK_SHIFT 24U

#define OS_PENDSV_PRIORITY 0xF0U
#define OS_SYSTICK_PRIORITY 0xE0U

#define OS_INVALID_TASK_INDEX UINT32_MAX

#define OS_STACK_FILL_PATTERN UINT32_C(0xA5A5A5A5)
#define OS_STACK_CANARY UINT32_C(0xDEADBEEF)

#define OS_INITIAL_TASK_FRAME_WORDS 16U

_Static_assert(
    OS_STACK_SIZE > OS_INITIAL_TASK_FRAME_WORDS,
    "Task stack must contain space for the initial frame and canary");

typedef enum
{
    OS_KERNEL_UNINITIALIZED = 0,
    OS_KERNEL_INITIALIZED,
    OS_KERNEL_RUNNING
} OsKernelState;

static volatile OsKernelState osKernelState = OS_KERNEL_UNINITIALIZED;

static uint32_t _taskStacks[OS_MAX_TASKS][OS_STACK_SIZE] __attribute__((aligned(8))); /* Static stack storage for all tasks */

static volatile uint8_t _taskCount = 0; /* Number of tasks created so far */
extern volatile uint32_t _osTick;
volatile uint32_t osNextTask = 0; /* Index of next task to run (set by scheduler) */
static uint32_t _idleTaskIndex = OS_INVALID_TASK_INDEX;
volatile uint32_t osStackOverflowTask = OS_INVALID_TASK_INDEX;

volatile TCB_t _tcbs[OS_MAX_TASKS];
volatile uint32_t osCurrentTask = 0U;

static OsStatus createTaskInternal(void (*taskFunc)(void *), void *arg);

bool osKernelIsRunning(void)
{
    return (osKernelState == OS_KERNEL_RUNNING);
}

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

    // fill stack with fill pattern
    for (uint32_t i = 0U; i < OS_STACK_SIZE; i++)
    {
        stack[i] = OS_STACK_FILL_PATTERN;
    }

    stack[0] = OS_STACK_CANARY; // the lowest portion of the stack will have the canary value to indicate end of stack
    /* IOW Corruption of this word indicates that the stack reached its limit. */

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

OsStatus osKernelInit(void)
{

    if (osIsInInterruptContext())
    {
        return OS_ERROR_ISR_CONTEXT; // kernel init modifies tcb and task stack therefore must run in thread mode
    }

    if (osKernelState != OS_KERNEL_UNINITIALIZED)
    {
        return OS_ERROR_INVALID_STATE;
    }

    /* Clear any pending SysTick or PendSV exceptions */
    osClearPendingSchedulerExceptions();

    /*
     * SysTick runs before PendSV and PendSV is lowest-priority kernel exception.
     */
    osConfigureExceptionPriorities();

    _taskCount = 0;
    osCurrentTask = 0;
    osNextTask = 0;

    for (uint32_t i = 0U; i < OS_MAX_TASKS; i++)
    {
        _tcbs[i].stackPtr = NULL;
        _tcbs[i].state = TASK_TERMINATED;
        _tcbs[i].waitReason = TASK_WAIT_NONE;
        _tcbs[i].delayTicks = 0U;
    }
    memset(_taskStacks, 0, sizeof(_taskStacks));

    _idleTaskIndex = _taskCount; /* First task is the idle task */
    OsStatus idleStatus = createTaskInternal(osIdleTask, NULL);

    if (idleStatus != OS_OK)
    {
        return idleStatus;
    }

    _tcbs[_idleTaskIndex].state = TASK_RUNNING;
    osKernelState = OS_KERNEL_INITIALIZED;
    return OS_OK;
}

OsStatus osTaskCreate(void (*taskFunc)(void *), void *arg)
{
    if (osIsInInterruptContext())
    {
        return OS_ERROR_ISR_CONTEXT;
    }

    if (osKernelState != OS_KERNEL_INITIALIZED)
    {
        return OS_ERROR_INVALID_STATE;
    }
    return createTaskInternal(taskFunc, arg);
}
static OsStatus createTaskInternal(void (*taskFunc)(void *), void *arg)
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

static void osHandleStackOverflow(uint32_t taskIndex)
{
    __asm volatile("CPSID I" ::: "memory");

    osStackOverflowTask = taskIndex;

    __asm volatile("DSB" ::: "memory");

    while (1)
    {
        __asm volatile("WFI");
    }
}

static void osCheckTaskStacks(void)
{
    for (uint8_t i = 0; i < _taskCount; i++)
    {
        if (_taskStacks[i][0] != OS_STACK_CANARY)
        {
            osHandleStackOverflow(i);
        }
    }
}

void osScheduler(void)
{
    if (_taskCount == 0U)
    {
        return;
    }

    osCheckTaskStacks();

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
    if (osKernelState != OS_KERNEL_RUNNING)
    {
        return;
    }

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
        osRequestContextSwitch();
    }
}

OsStatus osTaskDelay(uint32_t ticks)
{
    if (ticks == 0U)
    {
        return OS_OK;
    }

    if (!osKernelIsRunning())
    {
        return OS_ERROR_INVALID_STATE;
    }

    if (osIsInInterruptContext())
    {
        return OS_ERROR_ISR_CONTEXT; // executing in handler mode. ISR can't amke blocking calls.
    }

    if (osAreInterruptsDisabled())
    {
        return OS_ERROR_INTERRUPTS_DISABLED; // interrupts already disabled in caller
    }

    uint32_t irqState = osIrqSave();

    _tcbs[osCurrentTask].delayTicks = ticks;
    _tcbs[osCurrentTask].state = TASK_BLOCKED;
    _tcbs[osCurrentTask].waitReason = TASK_WAIT_DELAY;

    osScheduler();

    osRequestContextSwitch(); /* PendSV set pending */

    osIrqRestore(irqState); /* Return interrupt Enable/Disable back to original state */

    return OS_OK;
}

__attribute__((naked, noreturn)) static void startFirstTask(void)
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

OsStatus osKernelStart(void)
{
    uint32_t irqState = osIrqSave();

    if (osIsInInterruptContext())
    {
        osIrqRestore(irqState);
        return OS_ERROR_ISR_CONTEXT;
    }

    if (osKernelState != OS_KERNEL_INITIALIZED)
    {
        osIrqRestore(irqState);
        return OS_ERROR_INVALID_STATE;
    }
    osKernelState = OS_KERNEL_RUNNING;
    startFirstTask();
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

OsStatus osTaskGetStackHighWaterMark(uint32_t taskIndex, uint32_t *peakUsedWords)
{
    if (taskIndex >= _taskCount)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (peakUsedWords == NULL)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t irqState = osIrqSave();

    if (_taskStacks[taskIndex][0] != OS_STACK_CANARY)
    {
        osIrqRestore(irqState);
        return OS_ERROR_STACK_OVERFLOW;
    }
    uint32_t firstUsedIndex = 1U;
    while (
        (firstUsedIndex < OS_STACK_SIZE) &&
        (_taskStacks[taskIndex][firstUsedIndex] == OS_STACK_FILL_PATTERN))

    {
        firstUsedIndex++;
    }

    *peakUsedWords = OS_STACK_SIZE - firstUsedIndex;

    osIrqRestore(irqState);

    return OS_OK;
}
