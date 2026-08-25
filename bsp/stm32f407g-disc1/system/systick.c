#include "systick.h"
#include <stdint.h>

#define SYST_CSR (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014U)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018U)
#define SYST_CALIB (*(volatile uint32_t *)0xE000E01CU)

#define CPU_CLOCK_HZ 16000000UL

volatile uint32_t _osTick = 0;

void SysTick_Init(uint32_t tickRateHz)
{
    if (tickRateHz == 0U)
    {
        return;
    }

    uint32_t reloadVal =
        (CPU_CLOCK_HZ / tickRateHz) - 1U;

    SYST_CSR = 0U; // Disable SysTick while changing its configuration.

    SYST_RVR = reloadVal; // configure reload cycle

    SYST_CVR = 0U; // cear current counter val

    SYST_CSR =
        (1U << 2) | // use processor clock.
        (1U << 1) | // enable SysTick exception.
        (1U << 0);  // enable the counter.
}
uint32_t osGetTick(void)

{
    return _osTick;
}
