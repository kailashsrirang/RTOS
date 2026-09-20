#include <stdint.h>
#include "uart.h"

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

/* ===== init ===== */
void uart4_init(void)
{
    RCC_AHB1ENR |= (1U << 0);  // GPIOA clock enable
    RCC_APB1ENR |= (1U << 19); // UART4 clock enable

    /* PA0, PA1 -> alternate function */
    GPIOA_MODER &= ~(0xFU);
    GPIOA_MODER |= (0xAU);

    GPIOA_PUPDR &= ~(0xFU);
    GPIOA_PUPDR |= (0x5U);

    /* AF8 for PA0/PA1 */
    GPIOA_AFRL &= ~(0xFFU);
    GPIOA_AFRL |= (0x88U);

    /* baud rate */
    UART4_BRR = 0x683; // your chosen value

    /* enable TX, RX, UART */
    UART4_CR1 |= (1U << 3);  // TE
    UART4_CR1 |= (1U << 2);  // RE
    UART4_CR1 |= (1U << 13); // UE
}

/* ===== RX ===== */
uint8_t uart4_read(void)
{
    while (!(UART4_SR & (1U << 5)))
    {
        // RXNE
    }

    return (uint8_t)UART4_DR;
}

/* ===== TX ===== */
void uart4_write(uint8_t data)
{
    while (!(UART4_SR & (1U << 7)))
    {
        // TXE
    }

    UART4_DR = data;
}

/* ===== print ===== */
void uart4_print(const char *s)
{
    while (*s)
    {
        uart4_write((uint8_t)*s++);
    }
}

void uart4_println(const char *s)
{
    uart4_print(s);
    uart4_write('\r');
    uart4_write('\n');
}
