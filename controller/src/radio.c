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

#include <ndl3.h>

#include "inc/radio.h"

#include "inc/task.h"

#include "inc/driver_glue.h"
#include "inc/uart_serial_driver.h"
#include "inc/led_driver.h"
#include "inc/xbee_framing.h"

#include "legacy_piemos_framing.h"   // NOLINT(build/include)


static portTASK_FUNCTION_PROTO(radioNewTask, pvParameters);

BaseType_t radioInit() {
  radio_driver_init();

  return xTaskCreate(radioNewTask, "Runtime", 1024, NULL, tskIDLE_PRIORITY,
                     NULL);
}

// TODO(cduck): Move this to uart_serial_driver.c and make it work
int radio_uart_serial_send_and_finish_data(const uint8_t *data, size_t len) {
  // TODO(rqou): Asynchronous?
  // TODO(rqou): Error handling
  void *txn = uart_serial_send_data(radio_driver, data, len);
  while ((uart_serial_send_status(radio_driver, txn) !=
      UART_SERIAL_SEND_DONE) &&
      (uart_serial_send_status(radio_driver, txn) !=
        UART_SERIAL_SEND_ERROR)) {}
  return uart_serial_send_finish(radio_driver, txn);
}



static portTASK_FUNCTION_PROTO(radioNewTask, pvParameters) {
  (void) pvParameters;

  NDL3Net * target = NDL3_new(pvPortMalloc, vPortFree, NULL);
  NDL3_open(target, NDL3_UBJSON_PORT);
  NDL3_open(target, NDL3_STRING_PORT);
  NDL3_open(target, NDL3_CODE_PORT);
  char * in_msg = "This string came from the Big Board:  ";
  NDL3_send(target, 1, in_msg, 1 + strlen(in_msg));
  char * recvMsg = NULL;

  const uint8_t prefixLen = 1;
  uint8_t buffer[256];
  buffer[0] = NDL3_IDENT;
  uint8_t *ndlBuffer = buffer+prefixLen;
  NDL3_size popSize = 0;
  NDL3_size recvSize = 0;
  NDL3_size uartRecvSize = 0;
  int a = 0;
  while (1) {
    NDL3_recv(target, NDL3_UBJSON_PORT, (void **) &recvMsg, &recvSize);
    // Do stuff with recieved message
    if (recvSize >= 1) {
      led_driver_set_mode(PATTERN_JUST_RED);
      led_driver_set_fixed(recvMsg[0], 0b111);
    }
    vPortFree(recvMsg);

    NDL3_recv(target, NDL3_STRING_PORT, (void **) &recvMsg, &recvSize);
    // Do stuff with recieved message

    vPortFree(recvMsg);

    NDL3_recv(target, NDL3_CODE_PORT, (void **) &recvMsg, &recvSize);
    // Do stuff with recieved message

    vPortFree(recvMsg);


    if (a % 1000 == 0) {
      // Send message
      in_msg[strlen(in_msg) - 1] = (char) (a/1000%10) + '0';
      NDL3_send(target, NDL3_STRING_PORT, in_msg, 1 + strlen(in_msg));
    }

    uint8_t *recvUart = uart_serial_receive_packet(radio_driver,
      &uartRecvSize, 0);
    if (recvUart) {
      if (uartRecvSize >= prefixLen && recvUart[0] == NDL3_IDENT) {
        NDL3_L2_push(target, recvUart, uartRecvSize-prefixLen);
      }
      vPortFree(recvUart);
    }

    NDL3_L2_pop(target, ndlBuffer, sizeof(buffer)-prefixLen, &popSize);
    radio_uart_serial_send_and_finish_data(buffer, popSize+prefixLen);

    NDL3_elapse_time(target, 1);

    a++;
  }
}
