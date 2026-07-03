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

    uint32_t reloadVal = (CPU_CLOCK_HZ / tickRateHz) - 1;
    SYST_CSR = (1U << 2) | (1U << 1) | (1U << 0); // bit2=processor clock, bit1=enable interrupt, bit0=enable timer
    SYST_RVR = reloadVal;

    SYST_CVR = 0; // clear current value
}

uint32_t osGetTick(void)

{
    return _osTick;
}
