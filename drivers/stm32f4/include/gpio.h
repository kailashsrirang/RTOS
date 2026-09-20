#ifndef STM32F4_GPIO_H
#define STM32F4_GPIO_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define AHB1_PERIPHERAL_BASE 0x40020000UL

#define GPIOA_BASE (AHB1_PERIPHERAL_BASE + 0x0000UL)
#define GPIOB_BASE (AHB1_PERIPHERAL_BASE + 0x0400UL)
#define GPIOC_BASE (AHB1_PERIPHERAL_BASE + 0x0800UL)
#define GPIOD_BASE (AHB1_PERIPHERAL_BASE + 0x0C00UL)
#define GPIOE_BASE (AHB1_PERIPHERAL_BASE + 0x1000UL)
#define GPIOF_BASE (AHB1_PERIPHERAL_BASE + 0x1400UL)
#define GPIOG_BASE (AHB1_PERIPHERAL_BASE + 0x1800UL)
#define GPIOH_BASE (AHB1_PERIPHERAL_BASE + 0x1C00UL)
#define GPIOI_BASE (AHB1_PERIPHERAL_BASE + 0x2000UL)

#define GPIOA ((GpioRegisters *)GPIOA_BASE)
#define GPIOB ((GpioRegisters *)GPIOB_BASE)
#define GPIOC ((GpioRegisters *)GPIOC_BASE)
#define GPIOD ((GpioRegisters *)GPIOD_BASE)
#define GPIOE ((GpioRegisters *)GPIOE_BASE)
#define GPIOF ((GpioRegisters *)GPIOF_BASE)
#define GPIOG ((GpioRegisters *)GPIOG_BASE)
#define GPIOH ((GpioRegisters *)GPIOH_BASE)
#define GPIOI ((GpioRegisters *)GPIOI_BASE)

#define RCC_BASE (AHB1_PERIPHERAL_BASE + 0x3800UL)
#define RCC ((RccRegisters *)RCC_BASE)

#define GPIO_PIN_COUNT 16U
#define GPIO_PORT_SPACING 0x0400UL

typedef struct
{
    volatile uint32_t MODER;   /* Offset 0x00 */
    volatile uint32_t OTYPER;  /* Offset 0x04 */
    volatile uint32_t OSPEEDR; /* Offset 0x08 */
    volatile uint32_t PUPDR;   /* Offset 0x0C */
    volatile uint32_t IDR;     /* Offset 0x10 */
    volatile uint32_t ODR;     /* Offset 0x14 */
    volatile uint32_t BSRR;    /* Offset 0x18 */
    volatile uint32_t LCKR;    /* Offset 0x1C */
    volatile uint32_t AFR[2];  /* Offsets 0x20 and 0x24 */
} GpioRegisters;

typedef enum
{
    GPIO_MODE_INPUT = 0U,
    GPIO_MODE_OUTPUT = 1U,
    GPIO_MODE_ALTERNATE = 2U,
    GPIO_MODE_ANALOG = 3U
} GpioMode;

typedef struct
{
    volatile uint32_t CR;       /* Offset 0x00 */
    volatile uint32_t PLLCFGR;  /* Offset 0x04 */
    volatile uint32_t CFGR;     /* Offset 0x08 */
    volatile uint32_t CIR;      /* Offset 0x0C */
    volatile uint32_t AHB1RSTR; /* Offset 0x10 */
    volatile uint32_t AHB2RSTR; /* Offset 0x14 */
    volatile uint32_t AHB3RSTR; /* Offset 0x18 */
    uint32_t RESERVED0;         /* Offset 0x1C */
    volatile uint32_t APB1RSTR; /* Offset 0x20 */
    volatile uint32_t APB2RSTR; /* Offset 0x24 */
    uint32_t RESERVED1[2];      /* Offsets 0x28–0x2C */
    volatile uint32_t AHB1ENR;  /* Offset 0x30 */
} RccRegisters;

typedef enum
{
    GPIO_OUTPUT_PUSH_PULL = 0U,
    GPIO_OUTPUT_OPEN_DRAIN = 1U
} GpioOutputType;

typedef enum
{
    GPIO_SPEED_LOW = 0U,
    GPIO_SPEED_MEDIUM = 1U,
    GPIO_SPEED_FAST = 2U,
    GPIO_SPEED_HIGH = 3U
} GpioSpeed;

typedef enum
{
    GPIO_PULL_NONE = 0U,
    GPIO_PULL_UP = 1U,
    GPIO_PULL_DOWN = 2U
} GpioPull;

/* GPIO Assertions */
_Static_assert(offsetof(GpioRegisters, MODER) == 0x00U,
               "Incorrect GPIO MODER offset");

_Static_assert(offsetof(GpioRegisters, OTYPER) == 0x04U,
               "Incorrect GPIO OTYPER offset");

_Static_assert(offsetof(GpioRegisters, OSPEEDR) == 0x08U,
               "Incorrect GPIO OSPEEDR offset");

_Static_assert(offsetof(GpioRegisters, PUPDR) == 0x0CU,
               "Incorrect GPIO PUPDR offset");

_Static_assert(offsetof(GpioRegisters, IDR) == 0x10U,
               "Incorrect GPIO IDR offset");

_Static_assert(offsetof(GpioRegisters, ODR) == 0x14U,
               "Incorrect GPIO ODR offset");

_Static_assert(offsetof(GpioRegisters, BSRR) == 0x18U,
               "Incorrect GPIO BSRR offset");

_Static_assert(offsetof(GpioRegisters, LCKR) == 0x1CU,
               "Incorrect GPIO LCKR offset");

_Static_assert(offsetof(GpioRegisters, AFR) == 0x20U,
               "Incorrect GPIO AFR offset");

_Static_assert(sizeof(GpioRegisters) == 0x28U,
               "Incorrect GPIO register structure size");

/* RCC Assertions */
_Static_assert(offsetof(RccRegisters, CR) == 0x00U,
               "Incorrect RCC CR offset");

_Static_assert(offsetof(RccRegisters, PLLCFGR) == 0x04U,
               "Incorrect RCC PLLCFGR offset");

_Static_assert(offsetof(RccRegisters, CFGR) == 0x08U,
               "Incorrect RCC CFGR offset");

_Static_assert(offsetof(RccRegisters, CIR) == 0x0CU,
               "Incorrect RCC CIR offset");

_Static_assert(offsetof(RccRegisters, AHB1RSTR) == 0x10U,
               "Incorrect RCC AHB1RSTR offset");

_Static_assert(offsetof(RccRegisters, AHB2RSTR) == 0x14U,
               "Incorrect RCC AHB2RSTR offset");

_Static_assert(offsetof(RccRegisters, AHB3RSTR) == 0x18U,
               "Incorrect RCC AHB3RSTR offset");

_Static_assert(offsetof(RccRegisters, RESERVED0) == 0x1CU,
               "Incorrect RCC RESERVED0 offset");

_Static_assert(offsetof(RccRegisters, APB1RSTR) == 0x20U,
               "Incorrect RCC APB1RSTR offset");

_Static_assert(offsetof(RccRegisters, APB2RSTR) == 0x24U,
               "Incorrect RCC APB2RSTR offset");

_Static_assert(offsetof(RccRegisters, RESERVED1) == 0x28U,
               "Incorrect RCC RESERVED1 offset");

_Static_assert(offsetof(RccRegisters, AHB1ENR) == 0x30U,
               "Incorrect RCC AHB1ENR offset");

_Static_assert(sizeof(RccRegisters) == 0x34U,
               "Incorrect RCC register structure size");
bool gpioEnableClock(GpioRegisters *gpio);
bool gpioSetMode(GpioRegisters *gpio, uint8_t pin, GpioMode mode);
bool gpioWritePin(GpioRegisters *gpio, uint8_t pin, bool value);
bool gpioTogglePin(GpioRegisters *gpio, uint8_t pin);
bool gpioReadPin(const GpioRegisters *gpio, uint8_t pin, bool *value);
bool gpioConfigureOutput(GpioRegisters *gpio, uint8_t pin, GpioOutputType type, GpioSpeed speed, GpioPull pull, bool initialValue);

bool gpioConfigureInput(GpioRegisters *gpio,
                        uint8_t pin,
                        GpioPull pull);

bool gpioSetAlternateFunction(GpioRegisters *gpio,
                              uint8_t pin,
                              uint8_t alternateFunction);
bool gpioConfigureAlternate(GpioRegisters *gpio,
                            uint8_t pin,
                            uint8_t alternateFunction,
                            GpioOutputType type,
                            GpioSpeed speed,
                            GpioPull pull);
#endif /* STM32F4_GPIO_H */