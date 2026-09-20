#include <stdint.h>
#include <os_kernel.h>
#include <stddef.h>

#define SCB_CFSR (*(volatile const uint32_t *)0xE000ED28UL)
#define SCB_HFSR (*(volatile const uint32_t *)0xE000ED2CUL)
#define SCB_MMFAR (*(volatile const uint32_t *)0xE000ED34UL)
#define SCB_BFAR (*(volatile const uint32_t *)0xE000ED38UL)

#define HARD_FAULT_RECORD_MAGIC UINT32_C(0x48465254)

#define STM32F407_VECTOR_COUNT 98U
#define WEAK_DEFAULT_HANDLER(name) void name(void) __attribute__((weak, alias("Default_Handler")))

typedef void (*VectorHandler)(void);

extern int main(void);

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;
extern volatile uint32_t osCurrentTask;

typedef void (*VectorHandler)(void);

void Default_Handler(void);
void HardFault_Handler(void) __attribute__((naked, noreturn));
void Reset_Handler(void) __attribute__((noreturn));

void Default_Handler(void)
{
    while (1)
        ;
}
static void hardFaultCapture(const uint32_t *stackFrame, uint32_t exceptionReturn)
    __attribute__((used, noinline, noreturn)); // used: tells compiler this is a used fucntion dont deete. noinline: keep as seperate funciotn. noreturn: fucniton will not return

/*Vector Table Routines */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler"))); // use Default_Handler if handlers are not defined
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((naked, noreturn)) void HardFault_Handler(void)
{
    __asm volatile(
        "TST   LR, #4            \n"   // LR & 0b100
        "ITE   EQ                \n"   // if == exec next func
        "MRSEQ R0, MSP           \n"   // r0=msp if
        "MRSNE R0, PSP           \n"   // else ro=psp
        "MOV   R1, LR            \n"   // save LR to R1
        "B     hardFaultCapture  \n"); // exec hard fault capture fucntion
}
// we pass the pointer to the correct stack to C funciton bc the values in the stack contain info about the ahard fault
// r0 is first arg passed ot function and r1 is second

WEAK_DEFAULT_HANDLER(WWDG_IRQHandler); // expands to : void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
WEAK_DEFAULT_HANDLER(PVD_IRQHandler);
WEAK_DEFAULT_HANDLER(TAMP_STAMP_IRQHandler);
WEAK_DEFAULT_HANDLER(RTC_WKUP_IRQHandler);
WEAK_DEFAULT_HANDLER(FLASH_IRQHandler);
WEAK_DEFAULT_HANDLER(RCC_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI0_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI1_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI2_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI3_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI4_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream0_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream1_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream2_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream3_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream4_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream5_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream6_IRQHandler);
WEAK_DEFAULT_HANDLER(ADC_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN1_TX_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN1_RX0_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN1_RX1_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN1_SCE_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI9_5_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM1_BRK_TIM9_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM1_UP_TIM10_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM1_TRG_COM_TIM11_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM1_CC_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM2_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM3_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM4_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C1_ER_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C2_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C2_ER_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI1_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI2_IRQHandler);
WEAK_DEFAULT_HANDLER(USART1_IRQHandler);
WEAK_DEFAULT_HANDLER(USART2_IRQHandler);
WEAK_DEFAULT_HANDLER(USART3_IRQHandler);
WEAK_DEFAULT_HANDLER(EXTI15_10_IRQHandler);
WEAK_DEFAULT_HANDLER(RTC_Alarm_IRQHandler);
WEAK_DEFAULT_HANDLER(OTG_FS_WKUP_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM8_BRK_TIM12_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM8_UP_TIM13_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM8_TRG_COM_TIM14_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM8_CC_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA1_Stream7_IRQHandler);
WEAK_DEFAULT_HANDLER(FSMC_IRQHandler);
WEAK_DEFAULT_HANDLER(SDIO_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM5_IRQHandler);
WEAK_DEFAULT_HANDLER(SPI3_IRQHandler);
WEAK_DEFAULT_HANDLER(UART4_IRQHandler);
WEAK_DEFAULT_HANDLER(UART5_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM6_DAC_IRQHandler);
WEAK_DEFAULT_HANDLER(TIM7_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream0_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream1_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream2_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream3_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream4_IRQHandler);
WEAK_DEFAULT_HANDLER(ETH_IRQHandler);
WEAK_DEFAULT_HANDLER(ETH_WKUP_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN2_TX_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN2_RX0_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN2_RX1_IRQHandler);
WEAK_DEFAULT_HANDLER(CAN2_SCE_IRQHandler);
WEAK_DEFAULT_HANDLER(OTG_FS_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream5_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream6_IRQHandler);
WEAK_DEFAULT_HANDLER(DMA2_Stream7_IRQHandler);
WEAK_DEFAULT_HANDLER(USART6_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C3_EV_IRQHandler);
WEAK_DEFAULT_HANDLER(I2C3_ER_IRQHandler);
WEAK_DEFAULT_HANDLER(OTG_HS_EP1_OUT_IRQHandler);
WEAK_DEFAULT_HANDLER(OTG_HS_EP1_IN_IRQHandler);
WEAK_DEFAULT_HANDLER(OTG_HS_WKUP_IRQHandler);
WEAK_DEFAULT_HANDLER(OTG_HS_IRQHandler);
WEAK_DEFAULT_HANDLER(DCMI_IRQHandler);
WEAK_DEFAULT_HANDLER(CRYP_IRQHandler);
WEAK_DEFAULT_HANDLER(HASH_RNG_IRQHandler);
WEAK_DEFAULT_HANDLER(FPU_IRQHandler);

__attribute__((noreturn)) void Reset_Handler(void)
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

    // main must never return. if it does. trap it here in infinite loop
    __asm volatile("CPSID I" ::: "memory");

    while (1)
    {
        __asm volatile("WFI");
    }
}

typedef struct
{
    uintptr_t initalStackPtr;
    VectorHandler handlers[STM32F407_VECTOR_COUNT - 1U];

} VectorTable;

__attribute__((used, section(".isr_vector"), aligned(512)))
const VectorTable vectorTable = {
    .initalStackPtr = (uintptr_t)&_estack,
    .handlers = {
        Reset_Handler,
        NMI_Handler,
        HardFault_Handler,
        MemManage_Handler,
        BusFault_Handler,
        UsageFault_Handler,
        0,
        0,
        0,
        0,
        SVC_Handler,
        DebugMon_Handler,
        0,
        PendSV_Handler,
        SysTick_Handler,
        WWDG_IRQHandler,               /* IRQ 0 */
        PVD_IRQHandler,                /* IRQ 1 */
        TAMP_STAMP_IRQHandler,         /* IRQ 2 */
        RTC_WKUP_IRQHandler,           /* IRQ 3 */
        FLASH_IRQHandler,              /* IRQ 4 */
        RCC_IRQHandler,                /* IRQ 5 */
        EXTI0_IRQHandler,              /* IRQ 6 */
        EXTI1_IRQHandler,              /* IRQ 7 */
        EXTI2_IRQHandler,              /* IRQ 8 */
        EXTI3_IRQHandler,              /* IRQ 9 */
        EXTI4_IRQHandler,              /* IRQ 10 */
        DMA1_Stream0_IRQHandler,       /* IRQ 11 */
        DMA1_Stream1_IRQHandler,       /* IRQ 12 */
        DMA1_Stream2_IRQHandler,       /* IRQ 13 */
        DMA1_Stream3_IRQHandler,       /* IRQ 14 */
        DMA1_Stream4_IRQHandler,       /* IRQ 15 */
        DMA1_Stream5_IRQHandler,       /* IRQ 16 */
        DMA1_Stream6_IRQHandler,       /* IRQ 17 */
        ADC_IRQHandler,                /* IRQ 18 */
        CAN1_TX_IRQHandler,            /* IRQ 19 */
        CAN1_RX0_IRQHandler,           /* IRQ 20 */
        CAN1_RX1_IRQHandler,           /* IRQ 21 */
        CAN1_SCE_IRQHandler,           /* IRQ 22 */
        EXTI9_5_IRQHandler,            /* IRQ 23 */
        TIM1_BRK_TIM9_IRQHandler,      /* IRQ 24 */
        TIM1_UP_TIM10_IRQHandler,      /* IRQ 25 */
        TIM1_TRG_COM_TIM11_IRQHandler, /* IRQ 26 */
        TIM1_CC_IRQHandler,            /* IRQ 27 */
        TIM2_IRQHandler,               /* IRQ 28 */
        TIM3_IRQHandler,               /* IRQ 29 */
        TIM4_IRQHandler,               /* IRQ 30 */
        I2C1_EV_IRQHandler,            /* IRQ 31 */
        I2C1_ER_IRQHandler,            /* IRQ 32 */
        I2C2_EV_IRQHandler,            /* IRQ 33 */
        I2C2_ER_IRQHandler,            /* IRQ 34 */
        SPI1_IRQHandler,               /* IRQ 35 */
        SPI2_IRQHandler,               /* IRQ 36 */
        USART1_IRQHandler,             /* IRQ 37 */
        USART2_IRQHandler,             /* IRQ 38 */
        USART3_IRQHandler,             /* IRQ 39 */
        EXTI15_10_IRQHandler,          /* IRQ 40 */
        RTC_Alarm_IRQHandler,          /* IRQ 41 */
        OTG_FS_WKUP_IRQHandler,        /* IRQ 42 */
        TIM8_BRK_TIM12_IRQHandler,     /* IRQ 43 */
        TIM8_UP_TIM13_IRQHandler,      /* IRQ 44 */
        TIM8_TRG_COM_TIM14_IRQHandler, /* IRQ 45 */
        TIM8_CC_IRQHandler,            /* IRQ 46 */
        DMA1_Stream7_IRQHandler,       /* IRQ 47 */
        FSMC_IRQHandler,               /* IRQ 48 */
        SDIO_IRQHandler,               /* IRQ 49 */
        TIM5_IRQHandler,               /* IRQ 50 */
        SPI3_IRQHandler,               /* IRQ 51 */
        UART4_IRQHandler,              /* IRQ 52 */
        UART5_IRQHandler,              /* IRQ 53 */
        TIM6_DAC_IRQHandler,           /* IRQ 54 */
        TIM7_IRQHandler,               /* IRQ 55 */
        DMA2_Stream0_IRQHandler,       /* IRQ 56 */
        DMA2_Stream1_IRQHandler,       /* IRQ 57 */
        DMA2_Stream2_IRQHandler,       /* IRQ 58 */
        DMA2_Stream3_IRQHandler,       /* IRQ 59 */
        DMA2_Stream4_IRQHandler,       /* IRQ 60 */
        ETH_IRQHandler,                /* IRQ 61 */
        ETH_WKUP_IRQHandler,           /* IRQ 62 */
        CAN2_TX_IRQHandler,            /* IRQ 63 */
        CAN2_RX0_IRQHandler,           /* IRQ 64 */
        CAN2_RX1_IRQHandler,           /* IRQ 65 */
        CAN2_SCE_IRQHandler,           /* IRQ 66 */
        OTG_FS_IRQHandler,             /* IRQ 67 */
        DMA2_Stream5_IRQHandler,       /* IRQ 68 */
        DMA2_Stream6_IRQHandler,       /* IRQ 69 */
        DMA2_Stream7_IRQHandler,       /* IRQ 70 */
        USART6_IRQHandler,             /* IRQ 71 */
        I2C3_EV_IRQHandler,            /* IRQ 72 */
        I2C3_ER_IRQHandler,            /* IRQ 73 */
        OTG_HS_EP1_OUT_IRQHandler,     /* IRQ 74 */
        OTG_HS_EP1_IN_IRQHandler,      /* IRQ 75 */
        OTG_HS_WKUP_IRQHandler,        /* IRQ 76 */
        OTG_HS_IRQHandler,             /* IRQ 77 */
        DCMI_IRQHandler,               /* IRQ 78 */
        CRYP_IRQHandler,               /* IRQ 79 */
        HASH_RNG_IRQHandler,           /* IRQ 80 */
        FPU_IRQHandler,                /* IRQ 81 */

    },
};

_Static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "Cortex-M4 addresses must be 32 bits");
_Static_assert(sizeof(VectorHandler) == sizeof(uint32_t), "Cortex-M4 handler addresses must be 32 bits");
_Static_assert(offsetof(VectorTable, handlers) == sizeof(uint32_t), "Unexpected padding before vector handlers");
_Static_assert(sizeof(VectorTable) == (STM32F407_VECTOR_COUNT * sizeof(uint32_t)), "STM32F407 vector table has the wrong size");

typedef struct
{
    uint32_t stackedR0;
    uint32_t stackedR1;
    uint32_t stackedR2;
    uint32_t stackedR3;
    uint32_t stackedR12;
    uint32_t stackedLr;
    uint32_t stackedPc;
    uint32_t stackedXpsr;

    uint32_t exceptionReturn;
    uint32_t activeStackPointer;

    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t mmfar;
    uint32_t bfar;

    uint32_t currentTask;
    uint32_t magic;
} HardFaultRecord;
volatile HardFaultRecord gHardFaultRecord;

static void hardFaultCapture(const uint32_t *stackFrame, uint32_t exceptionReturn)
{

    __asm volatile("CPSID I" ::: "memory");

    gHardFaultRecord.magic = 0U; // indicates that hard fault record is incomplete
    gHardFaultRecord.stackedR0 = stackFrame[0];
    gHardFaultRecord.stackedR1 = stackFrame[1];
    gHardFaultRecord.stackedR2 = stackFrame[2];
    gHardFaultRecord.stackedR3 = stackFrame[3];
    gHardFaultRecord.stackedR12 = stackFrame[4];
    gHardFaultRecord.stackedLr = stackFrame[5];
    gHardFaultRecord.stackedPc = stackFrame[6];
    gHardFaultRecord.stackedXpsr = stackFrame[7];

    gHardFaultRecord.exceptionReturn = exceptionReturn;
    gHardFaultRecord.activeStackPointer = (uint32_t)stackFrame;

    gHardFaultRecord.cfsr = SCB_CFSR;
    gHardFaultRecord.hfsr = SCB_HFSR;
    gHardFaultRecord.mmfar = SCB_MMFAR;
    gHardFaultRecord.bfar = SCB_BFAR;

    gHardFaultRecord.currentTask = osCurrentTask;

    __asm volatile("DSB" ::: "memory");

    gHardFaultRecord.magic = HARD_FAULT_RECORD_MAGIC;

    __asm volatile("DSB" ::: "memory");

    while (1)
    {
        __asm volatile("WFI");
    }
}