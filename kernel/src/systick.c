#include "systick.h"
#include "os_status.h"
#include <stdint.h>

#define SYST_CSR (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014U)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018U)
#define SYST_CALIB (*(volatile uint32_t *)0xE000E01CU)

#define SYSTICK_RELOAD_MAX UINT32_C(0x00FFFFFF)

volatile uint32_t _osTick = 0;

OsStatus SysTick_Init(uint32_t cpuClockHz, uint32_t tickRateHz)
{
    if (tickRateHz == 0U || cpuClockHz == 0U)
    {
        return OS_ERROR_OUT_OF_RANGE;
    }

    if (tickRateHz > cpuClockHz)
    {
        return OS_ERROR_OUT_OF_RANGE;
    }

    uint32_t reloadValue = (cpuClockHz / tickRateHz) - 1U;

    if (reloadValue > SYSTICK_RELOAD_MAX)
    {
        return OS_ERROR_OUT_OF_RANGE;
    }

    SYST_CSR = 0U; // Disable SysTick while changing its configuration.

    SYST_RVR = reloadValue; // configure reload cycle

    SYST_CVR = 0U; // cear current counter val

    SYST_CSR =
        (1U << 2) | // use processor clock.
        (1U << 1) | // enable SysTick exception.
        (1U << 0);  // enable the counter.

    return OS_OK;
}
uint32_t osGetTick(void)

{
    return _osTick;
}
