#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "gpio.h"
#include "button.h"

bool boardButtonInit(void)
{
    if (!gpioEnableClock(GPIOA))
    {
        return false;
    }
    if (!gpioConfigureInput(GPIOA, BOARD_BUTTON_PIN, GPIO_PULL_NONE))
    {
        return false;
    }
    return true;
}
bool boardButtonRead(bool *pressed)
{
    if (pressed == NULL)
    {
        return false;
    }
    return gpioReadPin(GPIOA, BOARD_BUTTON_PIN, pressed);
}