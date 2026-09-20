#ifndef OS_INTERRUPT_H
#define OS_INTERRUPT_H

#include <stdint.h>
#include <stdbool.h>

#define SCB_ICSR (*(volatile uint32_t *)0xE000ED04UL)
#define ICSR_PENDSTCLR (1U << 25)
#define ICSR_PENDSVCLR (1U << 27)
#define ICSR_PENDSVSET (1U << 28)
/*
 * Save the current interrupt state, then disable configurable interrupts.
 *
 * Return value:
 *     0 = interrupts were enabled
 *     1 = interrupts were already disabled
 */
static inline uint32_t osIrqSave(void)
{
    uint32_t primask;

    __asm volatile(
        "MRS %0, PRIMASK \n"
        "CPSID I         \n"
        : "=r"(primask)
        :
        : "memory");

    return primask;
}

/*
 * Restore the interrupt state that was returned by osIrqSave().
 */
static inline void osIrqRestore(uint32_t primask)
{
    __asm volatile(
        "MSR PRIMASK, %0 \n"
        :
        : "r"(primask)
        : "memory");
}

static inline void osRequestContextSwitch(void)
{
    SCB_ICSR = ICSR_PENDSVSET;
}

static inline void osClearPendingSchedulerExceptions(void)
{
    SCB_ICSR = ICSR_PENDSTCLR | ICSR_PENDSVCLR;
}

static inline uint32_t osGetIpsr(void)
{
    uint32_t ipsr;

    __asm volatile(
        "MRS %0, IPSR"
        : "=r"(ipsr));
    return ipsr;
}

static inline bool osIsInInterruptContext(void)
{
    return ((osGetIpsr() & 0x1FF) != 0U);
}

static inline uint32_t osGetPrimask(void)
{
    uint32_t primask;

    __asm volatile(
        "MRS %0, PRIMASK"
        : "=r"(primask));
    return primask;
}

static inline bool osAreInterruptsDisabled(void)
{

    return ((osGetPrimask() & 1U) != 0U);
}

#endif