#include "grub.h"

#include <asm/io.h>
#include <asm/cpu.h>

#define UART_CLOCK 1843200

#define UART_LCR_DLAB (1 << 7) 
#define UART_LCR_8N1  ((1 << 1) | (1 << 0)) 

#define UART_FCR_ENABLE     (1 << 0)
#define UART_FCR_CLEAR_RX   (1 << 1)
#define UART_FCR_CLEAR_TX   (1 << 2)
#define UART_FCR_TRIGGER_14 ((1 << 7) | (1 << 6))

#define UART_MCR_DTR (1 << 0)
#define UART_MCR_RTS (1 << 1)
#define UART_MCR_OUT2 (1 << 3)

#define UART_LSR_THRE (1 << 5) 

enum {
    UART_REG_DATA = 0,
    UART_REG_IER  = 1,
    UART_REG_FCR  = 2,
    UART_REG_LCR  = 3,
    UART_REG_MCR  = 4,
    UART_REG_LSR  = 5,
};

struct uart_s {
  uint16_t port_addr;
  uint16_t baud_rate;
  uint8_t is_initialized;
};

static struct uart_s uart = { 0 };

static inline uint16_t uart_divisor(uint16_t baud_rate)
{
  return UART_CLOCK / (16 * baud_rate);
}

grub_err_t uart_init(uint16_t port_address, uint16_t baud_rate)
{
  if (uart.is_initialized)
    return GRUB_ERR_ALREADY_INITIALIZED;

  uint16_t divisor = uart_divisor(baud_rate);

  x86_outb(port_address + UART_REG_IER, 0);

  x86_outb(port_address + UART_REG_LCR, UART_LCR_DLAB);

  x86_outb(port_address + UART_REG_DATA, divisor & 0xFF);
  x86_outb(port_address + UART_REG_IER, divisor >> 8);

  x86_outb(port_address + UART_REG_LCR, UART_LCR_8N1);

  x86_outb(port_address + UART_REG_FCR, UART_FCR_ENABLE | UART_FCR_CLEAR_RX | UART_FCR_CLEAR_TX | UART_FCR_TRIGGER_14);

  x86_outb(port_address + UART_REG_MCR, UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2);

  uart.port_addr = port_address;
  uart.baud_rate = baud_rate;
  uart.is_initialized = 1;

  return GRUB_OK;
}

void uart_shutdown(void)
{
  if (!uart.is_initialized)
    return;

  x86_outb(uart.port_addr + UART_REG_IER, 0);
  x86_outb(uart.port_addr + UART_REG_MCR, 0);

  uart.is_initialized = 0;
}

grub_err_t uart_write_byte(int8_t data)
{
  if (!uart.is_initialized)
    return GRUB_ERR_NOT_INITIALIZED;

  while ((x86_inb(uart.port_addr + UART_REG_LSR) & UART_LSR_THRE) == 0)
    x86_cpu_pause();

  x86_outb(uart.port_addr + UART_REG_DATA, data);

  return GRUB_OK;
}

