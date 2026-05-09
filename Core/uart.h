#include <stdint.h>

/* ===== register definitions ===== */
#define RCC_BASE 0x40023800UL
#define GPIOA_BASE 0x40020000UL
#define UART4_BASE 0x40004C00UL

#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_PUPDR (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_AFRL (*(volatile uint32_t *)(GPIOA_BASE + 0x20))

#define UART4_SR (*(volatile uint32_t *)(UART4_BASE + 0x00))
#define UART4_DR (*(volatile uint32_t *)(UART4_BASE + 0x04))
#define UART4_BRR (*(volatile uint32_t *)(UART4_BASE + 0x08))
#define UART4_CR1 (*(volatile uint32_t *)(UART4_BASE + 0x0C))

void uart4_init(void);
void uart4_println(char *s);