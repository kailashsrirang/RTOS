#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#include "os_status.h"

/*Reload Value = (CPU Clock / Desired Tick Rate) - 1*/
OsStatus SysTick_Init(uint32_t cpuClockHz, uint32_t tickRateHz);
uint32_t osGetTick(void);

#endif