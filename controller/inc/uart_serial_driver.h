// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#ifndef INC_UART_SERIAL_DRIVER_H_
#define INC_UART_SERIAL_DRIVER_H_

#include <stdint.h>
#include <sys/types.h>
#include "inc/stm32f4xx.h"
#include "inc/FreeRTOS.h"

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

extern int uart_serial_send_and_finish_data(uart_serial_module *module,
  uint8_t *data, size_t len);
extern uint8_t *uart_serial_receive_packet(uart_serial_module *module,
  size_t *len_out, int shouldBlock);
extern uint8_t *uart_serial_receive_packet_timeout(uart_serial_module *module,
  size_t *len_out, TickType_t timeout);

extern int uart_bus_logic_level(uart_serial_module *module);

extern void uart_serial_handle_tx_dma_interrupt(uart_serial_module *module);
extern void uart_serial_handle_rx_dma_interrupt(uart_serial_module *module);
extern void uart_serial_handle_uart_interrupt(uart_serial_module *module);

#endif  // INC_UART_SERIAL_DRIVER_H_
