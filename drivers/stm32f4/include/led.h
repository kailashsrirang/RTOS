#ifndef BOARD_LED_H
#define BOARD_LED_H
#include <stdbool.h>

typedef enum
{
    BOARD_LED_ORANGE = 0U,
    BOARD_LED_GREEN,
    BOARD_LED_RED,
    BOARD_LED_BLUE,
    BOARD_LED_COUNT
} BoardLed;

bool boardLedInit(void);
bool boardLedSet(BoardLed led, bool on);
bool boardLedToggle(BoardLed led);

#endif