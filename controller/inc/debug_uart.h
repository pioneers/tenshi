// The debug UART task handles sending/receiving data to/from the debug UART.
// This is currently hardcoded to USART1.
// TODO(rqou): DMA, RX

#ifndef INC_DEBUG_UART_H_
#define INC_DEBUG_UART_H_

#include <stdlib.h>

extern void debug_uart_setup(void);

extern void debug_uart_tx(const char *str, size_t len);

#endif  // INC_DEBUG_UART_H_
