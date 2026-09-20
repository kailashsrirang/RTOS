#ifndef STM32F4_UART_H
#define STM32F4_UART_H

#include <stdint.h>

void uart4_init(void);
uint8_t uart4_read(void);
void uart4_write(uint8_t data);
void uart4_print(const char *text);
void uart4_println(const char *text);

#endif /* STM32F4_UART_H */