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

#include <stdio.h>
#include <string.h>

#include <ndl3.h>

#include "inc/radio.h"

#include "inc/task.h"
#include "inc/queue.h"

#include "inc/driver_glue.h"
#include "inc/uart_serial_driver.h"
#include "inc/led_driver.h"
#include "inc/xbee_framing.h"
#include "inc/radio_stdio.h"

#include "legacy_piemos_framing.h"   // NOLINT(build/include)


typedef struct {
  NDL3_port port;
  const char *str;
  size_t len;
} RadioMessage;

QueueHandle_t radioQueue = NULL;


// Hard coded
volatile uint64_t host_addr = 0;  // Bytes reversed
#define TOTAL_PACKET_SIZE 100
#define NDL3_PACKET_SIZE (TOTAL_PACKET_SIZE-21)


void radio_send_xbee(uint8_t *data, size_t len);
void *ndAlloc(NDL3_size size, void * userdata);
void ndFree(void * to_free, void * userdata);



// Redirect printf, etc. to the radio

// con_write
// devoptab_t
// consoleInit
/*
ssize_t nocash_write(struct _reent *r, int fd, const char *ptr, size_t len) {
  nocashWrite(ptr, len);
  return len;
}

ssize_t con_write(struct _reent *r, int fd, const char *ptr, size_t len) {
}

static const devoptab_t dotab_stdout = {
  "con",
  0,
  NULL,
  NULL,
  con_write,
  NULL,
  NULL,
  NULL
};


void consoleInit() {
  static uint8_t firstConsoleInit = true;

  if (firstConsoleInit) {
    devoptab_list[STD_OUT] = &dotab_stdout;
    devoptab_list[STD_ERR] = &dotab_stdout;
    setvbuf(stdout, NULL , _IONBF, 0);
    setvbuf(stderr, NULL , _IONBF, 0);
    firstConsoleInit = false;
  }
}
*/


static portTASK_FUNCTION_PROTO(radioNewTask, pvParameters);

BaseType_t radioInit() {
  radio_driver_init();
  radio_stdio_init();

  radioQueue = xQueueCreate(100, sizeof(RadioMessage));

  return xTaskCreate(radioNewTask, "Radio", 512, NULL, tskIDLE_PRIORITY,
                     NULL);
}
void radioPushUbjson(const char *ubjson, size_t len) {
  RadioMessage msg = {
    .port = NDL3_UBJSON_PORT,
    .str = ubjson,
    .len = len
  };
  xQueueSend(radioQueue, &msg, 0);
}
void radioPushString(const char *str, size_t len) {
  RadioMessage msg = {
    .port = NDL3_STRING_PORT,
    .str = str,
    .len = len
  };
  xQueueSend(radioQueue, &msg, 0);
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

void radio_send_xbee(uint8_t *data, size_t len) {
  if (len > NDL3_PACKET_SIZE) return;

  // TODO(cduck): Do Robert's TODOs below

  // TODO(rqou): Hack
  uint8_t txbuf[TOTAL_PACKET_SIZE];

  // Create magic struct
  xbee_api_packet *packetOut = (xbee_api_packet *)(txbuf);
  packetOut->xbee_api_magic = XBEE_MAGIC;
  uint16_t payloadLen = sizeof(xbee_tx64_header) + len+1;
  packetOut->length = __REV16(payloadLen);
  packetOut->payload.tx64.xbee_api_type = XBEE_API_TYPE_TX64;
  packetOut->payload.tx64.frameId = 0;
  packetOut->payload.tx64.xbee_dest_addr = host_addr;
  packetOut->payload.tx64.options = 0;

  packetOut->payload.tx64.data[0] = NDL3_IDENT;
  memcpy(&(packetOut->payload.tx64.data[1]), data, len);

  xbee_fill_checksum(packetOut);

  // TODO(rqou): Asynchronous?
  // TODO(rqou): Abstract away the +4 properly
  // TODO(rqou): Error handling
  void *txn = uart_serial_send_data(radio_driver, txbuf, payloadLen + 4);
  while (!uart_serial_send_finish(radio_driver, txn)) {}
}
void *ndAlloc(NDL3_size size, void * userdata) {
  (void) userdata;
  void *ret = malloc(size);
  while (!ret) {}
  return ret;
}
void ndFree(void * to_free, void * userdata) {
  (void) userdata;
  free(to_free);
}



static portTASK_FUNCTION_PROTO(radioNewTask, pvParameters) {
  (void) pvParameters;

  NDL3Net * target = NDL3_new(ndAlloc, ndFree, NULL);
  NDL3_open(target, NDL3_UBJSON_PORT);
  NDL3_open(target, NDL3_STRING_PORT);
  NDL3_open(target, NDL3_CODE_PORT);
  NDL3_open(target, NDL3_CONFIG_PORT);
  NDL3_open(target, NDL3_FAST_PORT);
  char * recvMsg = NULL;

  const uint8_t prefixLen = 1;
  // TODO(rqou): Ugly
  uint8_t recXbeePacket_buf[256];
  xbee_api_packet *recXbeePacket = (xbee_api_packet *)(recXbeePacket_buf);
  xbee_rx64_header *recXbeeHeader;
  uint8_t buffer[NDL3_PACKET_SIZE];
  NDL3_size popSize = 0;
  NDL3_size recvSize = 0;
  NDL3_size uartRecvSize = 0;

  portTickType time = xTaskGetTickCount();
  portTickType lastTime = time;

  while (1) {
    recvMsg = NULL;
    recvSize = 0;
    NDL3_recv(target, NDL3_UBJSON_PORT, (void **) &recvMsg, &recvSize);
    // Send UBJSON to runtime
    if (recvMsg && recvSize >= 1) {
      // Trust this to free recvMsg
      runtimeRecieveUbjson(recvMsg, recvSize);
    } else {
      free(recvMsg);
    }

    recvMsg = NULL;
    recvSize = 0;
    NDL3_recv(target, NDL3_FAST_PORT, (void **) &recvMsg, &recvSize);
    // Send fast UBJSON to runtime
    if (recvMsg && recvSize >= 1) {
      // Trust this to free recvMsg
      runtimeRecieveUbjson(recvMsg, recvSize);
    } else {
      free(recvMsg);
    }

    recvMsg = NULL;
    recvSize = 0;
    NDL3_recv(target, NDL3_STRING_PORT, (void **) &recvMsg, &recvSize);
    // Do stuff with recieved message

    free(recvMsg);

    recvMsg = NULL;
    recvSize = 0;
    NDL3_recv(target, NDL3_CODE_PORT, (void **) &recvMsg, &recvSize);
    // Send code to runtime
    if (recvMsg && recvSize >= 1) {
      // Trust this to free recvMsg
      runtimeRecieveCode(recvMsg, recvSize);
    } else {
      free(recvMsg);
    }

    recvMsg = NULL;
    recvSize = 0;
    NDL3_recv(target, NDL3_CONFIG_PORT, (void **) &recvMsg, &recvSize);
    // Send code to runtime
    if (recvMsg && recvSize >= 1) {
      // Trust this to free recvMsg
      // TODO(cduck): Do something
      printf("Got config data\n");
      free(recvMsg);
    } else {
      free(recvMsg);
    }


    NDL3_error err = NDL3_pop_error(target);
    if (err && err != 5 && err != 7) {
      printf("Radio error a%d\n", err);
    }


    // Send over NDL3
    RadioMessage msg;
    if (xQueueReceive(radioQueue, &msg, 0) == pdTRUE) {
      // Trust NDL3 to free the char*
      NDL3_send(target, msg.port, msg.str, msg.len);
    }

    err = NDL3_pop_error(target);
    if (err && err != 5 && err != 7) {
      printf("Radio error b%d\n", err);
    }

    uartRecvSize = sizeof(recXbeePacket_buf);
    int ret = uart_serial_receive_packet(radio_driver, recXbeePacket,
      &uartRecvSize, 0);
    if (!ret) {
      recXbeePacket->length = __REV16(recXbeePacket->length);
      recXbeeHeader = (xbee_rx64_header*)&(recXbeePacket->payload);
      if (recXbeeHeader->xbee_api_type == XBEE_API_TYPE_RX64) {
        host_addr = recXbeeHeader->xbee_src_addr;
        if (uartRecvSize >= prefixLen && recXbeeHeader->data[0] == NDL3_IDENT) {
          NDL3_L2_push(target, (uint8_t*)recXbeeHeader->data+1,
            recXbeePacket->length-sizeof(xbee_rx64_header)-prefixLen);
        }
      }
    }

    err = NDL3_pop_error(target);
    if (err && err != 5 && err != 7) {
      printf("Radio error c%d\n", err);
    }

    if (host_addr != 0 && uart_serial_packets_waiting(radio_driver) <= 1) {
      popSize = 0;
      NDL3_L2_pop(target, buffer, NDL3_PACKET_SIZE, &popSize);
      if (popSize > 0) {
        radio_send_xbee(buffer, popSize);
      }
    }

    err = NDL3_pop_error(target);
    if (err && err != 5 && err != 7) {
      printf("Radio error d%d\n", err);
    }

    time = xTaskGetTickCount();
    NDL3_elapse_time(target, time-lastTime);
    lastTime = time;
  }
}
