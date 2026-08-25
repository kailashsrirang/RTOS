#include <stdbool.h>

typedef enum
{
    BOARD_LED_ORANGE = 0U,
    BOARD_LED_GREEN,
    BOARD_LED_RED,
    BOARD_LED_BLUE,
    BOARD_LED_COUNT
} BoardLed;

static const uint8_t boardLedPins[] = {
    [BOARD_LED_ORANGE] = 13U,
    [BOARD_LED_GREEN] = 12U,
    [BOARD_LED_RED] = 14U,
    [BOARD_LED_BLUE] = 15U};

bool boardLedInit(void);
bool boardLedSet(BoardLed led, bool on);
bool boardLedToggle(BoardLed led);