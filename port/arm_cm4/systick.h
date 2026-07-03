#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/*Reload Value = (CPU Clock / Desired Tick Rate) - 1*/
void SysTick_Init(uint32_t tickRateHz);
uint32_t osGetTick(void);

#endif