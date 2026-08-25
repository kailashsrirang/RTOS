#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "gpio.h"
#include "led.h"

bool boardLedInit(void)
{
    if (!gpioEnableClock(GPIOD))
    {
        return false;
    }

    for (uint8_t led = 0U; led < BOARD_LED_COUNT; led++)
    {
        if (!gpioConfigureOutput(GPIOD, boardLedPins[(uint8_t)led], GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_LOW, GPIO_PULL_NONE, false))
        {
            return false;
        }
    }
    return true;
}
bool boardLedSet(BoardLed led, bool on)
{
    if ((uint32_t)led >= (uint32_t)BOARD_LED_COUNT)
    {
        return false;
    }
    return gpioWritePin(GPIOD, boardLedPins[(uint8_t)led], on);
}
bool boardLedToggle(BoardLed led)
{
    if ((uint32_t)led >= (uint32_t)BOARD_LED_COUNT)
    {
        return false;
    }
    return gpioTogglePin(GPIOD, boardLedPins[(uint8_t)led]);
}