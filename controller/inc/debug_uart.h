#ifndef INC_DEBUG_UART_H_
#define INC_DEBUG_UART_H_

#include <stddef.h>
#include <stdint.h>

#include "inc/stm32f4xx.h"

// This driver adds a debug port on the unused SPI connector. Note that despite
// being called "UART," this debug port is not actually a UART. It is actually
// a master SPI port with a wire protocol specified below. This will require
// an Arduino or other device to translate to/from a normal UART. The reason
// for this module is because there are no more hardware UARTs available on
// the board.

// This module does not use DMA but does use interrupts. Do not use this to
// transfer large amounts of data.

// A special wire protocol is required because SPI cannot receive without
// transmitting. The wire protocol is as naive as possible: 0x00 is ignored
// and 0x01 is used as an escape character. 0x01 xx represents the character
// (xx-1). Other bytes are passed through normally.

typedef struct tag_debug_uart_module {
  // No stuff here yet
} debug_uart_module;

// This function bypasses the normal infrastructure and does a dumb, blocking
// send. However, it most likely won't break ever and can be used in cases
// like exception handlers. However, note that it DOES NOT WORK when the
// non-dumb interrupt-based interface is also working.
extern void debug_uart_dumb_write(debug_uart_module *module,
  const char *buf, size_t len);

extern debug_uart_module *debug_uart_init(SPI_TypeDef *periph_base);
extern void debug_uart_tx(debug_uart_module *module,
  const char *buf, size_t len);
extern size_t debug_uart_rx(debug_uart_module *module,
  const char *buf, size_t buf_len);

extern void debug_uart_irq(debug_uart_module *module);

#endif  // INC_DEBUG_UART_H_
