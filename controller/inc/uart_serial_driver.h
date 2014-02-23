#ifndef INC_UART_SERIAL_DRIVER_H_
#define INC_UART_SERIAL_DRIVER_H_

#include <stdint.h>
#include <sys/types.h>
#include "inc/stm32f4xx.h"

#define UART_SERIAL_SEND_QUEUED   1
#define UART_SERIAL_SEND_SENDING  2
#define UART_SERIAL_SEND_DONE     3
#define UART_SERIAL_SEND_ERROR    4

typedef struct tag_uart_serial_module {
  uint32_t length_finder_state;
} uart_serial_module;

extern uart_serial_module *uart_serial_init_module(int uart_num,
  ssize_t (*length_finder_fn)(uart_serial_module *, uint8_t),
  void (*txen_fn)(int), int baud);
extern void *uart_serial_send_data(uart_serial_module *module, uint8_t *data,
  size_t len);
extern int uart_serial_send_status(uart_serial_module *module,
  void *transaction);
extern int uart_serial_send_finish(uart_serial_module *module,
  void *transaction);
extern uint8_t *uart_serial_receive_packet(uart_serial_module *module,
  size_t *len_out);

extern void uart_serial_handle_tx_dma_interrupt(uart_serial_module *module);
extern void uart_serial_handle_rx_dma_interrupt(uart_serial_module *module);
extern void uart_serial_handle_uart_interrupt(uart_serial_module *module);

#endif  // INC_UART_SERIAL_DRIVER_H_
