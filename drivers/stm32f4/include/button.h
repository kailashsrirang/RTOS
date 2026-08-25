#include <stdbool.h>

#define BOARD_BUTTON_PIN 0U

bool boardButtonInit(void);
bool boardButtonRead(bool *pressed);
