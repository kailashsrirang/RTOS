#ifndef BOARD_BUTTON_H
#define BOARD_BUTTON_H

#include <stdbool.h>

#define BOARD_BUTTON_PIN 0U

bool boardButtonInit(void);
bool boardButtonRead(bool *pressed);

#endif
