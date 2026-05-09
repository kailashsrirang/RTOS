#include <stdint.h>

extern int main(void);

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

void Default_Handler(void)
{
    while (1)
        ;
}

/*Vector Table Routines */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler"))); // use Default_Handler if handlers are not defined
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void Reset_Handler(void)
{
    uint32_t *src, *dst;

    /*  Copy .data section from Flash to RAM */
    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    /* Zero out .bss section in RAM */
    dst = &_sbss;
    while (dst < &_ebss)
    {
        *dst++ = 0;
    }
    /*Jump to main*/
    main();
}

__attribute__((section(".vectors")))
uint32_t vector_table[] = {
    (uint32_t)&_estack,            /* 0: Initial Stack Pointer */
    (uint32_t)&Reset_Handler,      /* 1: Reset Handler */
    (uint32_t)&NMI_Handler,        /* 2: NMI */
    (uint32_t)&HardFault_Handler,  /* 3: Hard Fault */
    (uint32_t)&MemManage_Handler,  /* 4: Memory Manage */
    (uint32_t)&BusFault_Handler,   /* 5: Bus Fault */
    (uint32_t)&UsageFault_Handler, /* 6: Usage Fault */
    0,
    0,
    0,
    0,                           /* 7-10: Reserved */
    (uint32_t)&SVC_Handler,      /* 11: SVCall */
    (uint32_t)&DebugMon_Handler, /* 12: Debug Monitor */
    0,                           /* 13: Reserved */
    (uint32_t)&PendSV_Handler,   /* 14: PendSV */
    (uint32_t)&SysTick_Handler,  /* 15: SysTick */
};