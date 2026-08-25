#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "gpio.h"

bool gpioEnableClock(GpioRegisters *gpio)
{
    if (gpio == NULL)
    {
        return false;
    }
    uintptr_t address = (uintptr_t)gpio;
    if ((GPIOA_BASE > address || address > GPIOI_BASE))
    {
        return false;
    }
    uintptr_t offset = (address - GPIOA_BASE);

    if ((offset % GPIO_PORT_SPACING != 0))
    {
        return false;
    }
    uint32_t port_idx = offset / GPIO_PORT_SPACING;

    uint32_t mask = 1U << port_idx;
    RCC->AHB1ENR |= mask;
    (void)RCC->AHB1ENR;
    return true;
}

bool gpioSetMode(GpioRegisters *gpio, uint8_t pin, GpioMode mode)
{
    if (gpio == NULL || pin >= GPIO_PIN_COUNT || (uint32_t)mode > (uint32_t)GPIO_MODE_ANALOG)
    {
        return false;
    }
    uint32_t shift = ((uint32_t)pin * 2U);
    uint32_t mask = 3U << shift;

    uint32_t value = gpio->MODER;
    value &= ~mask; // clear bits
    value |= (uint32_t)mode << shift;
    gpio->MODER = value;

    return true;
}

bool gpioWritePin(GpioRegisters *gpio, uint8_t pin, bool value)
{
    if (gpio == NULL || pin >= GPIO_PIN_COUNT)
    {
        return false;
    }
    uint32_t mask = 1U << (uint32_t)pin;

    if (value)
    {
        gpio->BSRR = mask; // Lower 16 bits set outputs
    }
    else
    {
        gpio->BSRR = mask << 16U; // Upper 16 bits reset outputs
    }

    return true;
}

bool gpioTogglePin(GpioRegisters *gpio, uint8_t pin)
{
    if (gpio == NULL || pin >= GPIO_PIN_COUNT)
    {
        return false;
    }
    uint32_t mask = 1U << (uint32_t)pin;

    if ((gpio->ODR & mask) != 0U)
    {
        gpio->BSRR = mask << 16U;
    }
    else
    {
        gpio->BSRR = mask;
    }

    return true;
}

bool gpioReadPin(const GpioRegisters *gpio, uint8_t pin, bool *value)
{
    if (gpio == NULL ||
        value == NULL ||
        pin >= GPIO_PIN_COUNT)
    {
        return false;
    }
    uint32_t mask = (1U << (uint32_t)pin);
    *value = (gpio->IDR & mask) != 0U;

    return true;
}

bool gpioConfigureOutput(GpioRegisters *gpio, uint8_t pin, GpioOutputType type, GpioSpeed speed, GpioPull pull, bool initialValue)
{
    if (gpio == NULL ||
        pin >= GPIO_PIN_COUNT ||
        (uint32_t)type > (uint32_t)GPIO_OUTPUT_OPEN_DRAIN ||
        (uint32_t)speed > (uint32_t)GPIO_SPEED_HIGH ||
        (uint32_t)pull > (uint32_t)GPIO_PULL_DOWN)
    {
        return false;
    }

    gpioWritePin(gpio, pin, initialValue);

    uint32_t shift = ((uint32_t)pin);
    uint32_t mask = (uint32_t)type << shift;
    uint32_t clear_mask = ~(1U << shift);
    uint32_t registerValue = gpio->OTYPER;
    registerValue &= clear_mask; // clear bits
    registerValue |= mask;
    gpio->OTYPER = registerValue;

    shift = ((uint32_t)pin * 2U);
    uint32_t speed_mask = (uint32_t)speed << shift;
    clear_mask = ~(3U << shift);
    registerValue = gpio->OSPEEDR;
    registerValue &= clear_mask; // clear bits
    registerValue |= speed_mask;
    gpio->OSPEEDR = registerValue;

    shift = ((uint32_t)pin * 2U);
    uint32_t pull_mask = (uint32_t)pull << shift;
    clear_mask = ~(3U << shift);
    registerValue = gpio->PUPDR;
    registerValue &= clear_mask; // clear bits
    registerValue |= pull_mask;
    gpio->PUPDR = registerValue;

    return gpioSetMode(gpio, pin, GPIO_MODE_OUTPUT);
}

bool gpioConfigureInput(GpioRegisters *gpio,
                        uint8_t pin,
                        GpioPull pull)
{

    if (gpio == NULL ||
        pin >= GPIO_PIN_COUNT ||
        (uint32_t)pull > (uint32_t)GPIO_PULL_DOWN)
    {
        return false;
    }

    uint32_t shift = ((uint32_t)pin * 2U);
    uint32_t pull_mask = (uint32_t)pull << shift;
    uint32_t clear_mask = ~(3U << shift);
    gpio->PUPDR &= clear_mask; // clear bits
    gpio->PUPDR |= pull_mask;

    return gpioSetMode(gpio, pin, GPIO_MODE_INPUT);
}

bool gpioSetAlternateFunction(GpioRegisters *gpio,
                              uint8_t pin,
                              uint8_t alternateFunction)
{
    if (gpio == NULL ||
        pin >= GPIO_PIN_COUNT ||
        alternateFunction > 15U)
    {
        return false;
    }
    uint8_t idx = pin / 8U;
    uint32_t shift = (pin % 8U) * 4U;
    uint32_t clear_mask = ~(15U << (shift));

    uint32_t registerValue = gpio->AFR[idx];

    registerValue &= clear_mask;
    registerValue |= (uint32_t)alternateFunction << shift;

    gpio->AFR[idx] = registerValue;

    return gpioSetMode(gpio, pin, GPIO_MODE_ALTERNATE);
}

bool gpioConfigureAlternate(GpioRegisters *gpio,
                            uint8_t pin,
                            uint8_t alternateFunction,
                            GpioOutputType type,
                            GpioSpeed speed,
                            GpioPull pull)
{
    if (gpio == NULL ||
        pin >= GPIO_PIN_COUNT ||
        (uint32_t)type > (uint32_t)GPIO_OUTPUT_OPEN_DRAIN ||
        (uint32_t)speed > (uint32_t)GPIO_SPEED_HIGH ||
        (uint32_t)pull > (uint32_t)GPIO_PULL_DOWN ||
        alternateFunction > 15U)
    {
        return false;
    }

    uint32_t shift = ((uint32_t)pin);
    uint32_t mask = (uint32_t)type << shift;
    uint32_t clear_mask = ~(1U << shift);
    uint32_t registerValue = gpio->OTYPER;
    registerValue &= clear_mask; // clear bits
    registerValue |= mask;
    gpio->OTYPER = registerValue;

    shift = ((uint32_t)pin * 2U);
    uint32_t speed_mask = (uint32_t)speed << shift;
    clear_mask = ~(3U << shift);
    registerValue = gpio->OSPEEDR;
    registerValue &= clear_mask; // clear bits
    registerValue |= speed_mask;
    gpio->OSPEEDR = registerValue;

    shift = ((uint32_t)pin * 2U);
    uint32_t pull_mask = (uint32_t)pull << shift;
    clear_mask = ~(3U << shift);
    registerValue = gpio->PUPDR;
    registerValue &= clear_mask; // clear bits
    registerValue |= pull_mask;
    gpio->PUPDR = registerValue;

    return gpioSetAlternateFunction(gpio,
                                    pin,
                                    alternateFunction);
}