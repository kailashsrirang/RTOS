#include "os_kernel.h"
#include <stdint.h>
#include <string.h>
#include "mem.h"
#include "uart.h"

#define ICSR (*(volatile uint32_t *)0xE000ED04UL)

/* Static stack storage for all tasks */
static uint32_t _taskStacks[OS_MAX_TASKS][OS_STACK_SIZE];

/* TCB array — one per task slot */
TCB_t _tcbs[OS_MAX_TASKS];

/* Number of tasks created so far */
static uint8_t _taskCount = 0;

extern volatile uint32_t _osTick;

/* Index of currently running task */
volatile uint32_t osCurrentTask = 0;

/* Index of next task to run (set by scheduler) */
volatile uint32_t osNextTask = 0;

static void osTaskExit(void)
{
    while (1)
    {
        /* Task should never return */
    }
}

static void _initTaskStack(uint8_t taskIndex, void (*taskFunc)(void *), void *arg)
{
    uint32_t *stack = _taskStacks[taskIndex];
    uint32_t stackTop = OS_STACK_SIZE;

    /* Simulate the hardware exception stack frame */
    /* Fill in reverse — we're placing items at top of stack going downward */

    stack[--stackTop] = (1U << 24);                /* xPSR: Thumb bit must be set */
    stack[--stackTop] = ((uint32_t)taskFunc) | 1U; /* PC: task entry point */
    // stack[--stackTop] = 0xFFFFFFFDUL;       /* LR: EXC_RETURN — return to thread mode using PSP */
    stack[--stackTop] = ((uint32_t)osTaskExit) | 1U; /* LR: EXC_RETURN — return to thread mode using PSP */
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

void osKernelInit(void)
{
    // ICSR = (1U << 25) | (1U << 27); // clear pending SysTick/PendSV

    _taskCount = 0;
    osCurrentTask = 0;
    osNextTask = 0;
    memset(_tcbs, 0, sizeof(_tcbs));
    memset(_taskStacks, 0, sizeof(_taskStacks));
}

void osTaskCreate(void (*taskFunc)(void *), void *arg)
{
    uint32_t taskIndex = _taskCount;
    _tcbs[taskIndex].state = TASK_READY;

    _initTaskStack(taskIndex, taskFunc, arg);

    _taskCount++;

    // char info[50];
    // snprintf(info, sizeof(info), "Task %d Created", taskIndex);
    // uart4_println(info);
}

void osScheduler()
{
    uint8_t next = (osCurrentTask + 1) % _taskCount;

    for (uint8_t i = 0; i < _taskCount; i++)
    {
        if (_tcbs[next].state == TASK_READY || _tcbs[next].state == TASK_RUNNING)
        {
            break;
        }
        next = (next + 1) % _taskCount;
    }

    osNextTask = next;
}

void SysTick_Handler(void)
{
    for (uint8_t i = 0; i < _taskCount; i++)
    {
        if (_tcbs[i].state == TASK_BLOCKED)
        {
            if (_tcbs[i].delayTicks > 0)
            {
                _tcbs[i].delayTicks--;
                if (_tcbs[i].delayTicks == 0)
                {
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

        ICSR = (1U << 28); /* PendSV set pending */
    }
}

void osTaskDelay(uint32_t ticks)
{
    if (ticks == 0)
    {
        return;
    }
    __asm volatile("CPSID I"); // disable interrupts immediately

    _tcbs[osCurrentTask].delayTicks = ticks;
    _tcbs[osCurrentTask].state = TASK_BLOCKED;

    ICSR = (1U << 28); /* PendSV set pending */

    __asm volatile("CPSIE I"); // enable interrupts immediately
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