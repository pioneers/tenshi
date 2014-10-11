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

// Interpreter
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>

#include "inc/radio.h"
#include "inc/runtime.h"
#include "inc/runtime_interface.h"

#include "inc/FreeRTOS.h"
#include "inc/button_driver.h"
#include "inc/driver_glue.h"
#include "inc/i2c_master.h"
#include "inc/led_driver.h"
#include "inc/malloc_lock.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"
#include "inc/core_cm4.h"
#include "inc/core_cmInstr.h"
#include "inc/task.h"
#include "inc/xbee_framing.h"
#include "inc/smartsensor/smartsensor.h"

#include "legacy_piemos_framing.h"   // NOLINT(build/include)
#include "ngl_types.h"   // NOLINT(build/include)



uint8_t *code_buffer;
uint32_t code_buffer_len;

// TODO(rqou): This really doesn't go here.
int8_t PiEMOSAnalogVals[7];
uint8_t PiEMOSDigitalVals[8];

// TODO(rqou): Wat r abstraction?
// TODO(rqou): My macro-fu is not up to par
extern ngl_error *ngl_set_motor(ngl_float motor, ngl_float val);
static int lua_set_motor_old(lua_State *L) {
  ngl_error *ret = ngl_set_motor(lua_tonumber(L, 1), lua_tonumber(L, 2));
  // TODO(rqou): Um, I don't think this is how it's supposed to work?
  lua_pushnumber(L, ret == ngl_ok);
  return 1;
}
extern ngl_error *ngl_get_sensor(ngl_float sensor, ngl_float *val);
static int lua_get_sensor(lua_State *L) {
  ngl_float out;
  ngl_error *ret = ngl_get_sensor(lua_tonumber(L, 1), &out);
  // TODO(rqou): Um, I don't think this is how it's supposed to work?
  lua_pushnumber(L, out);
  return 1;
}

static portTASK_FUNCTION_PROTO(angelicTask, pvParameters) {
  (void) pvParameters;

  #ifndef TEST_STATIC_LUA
  if (*(uint32_t *)(code_buffer) == NGL_PACKAGE_MAGIC) {
  #else
  if (0) {
  #endif
    // Load Angelic blob
    ngl_buffer *program = ngl_buffer_alloc(code_buffer_len);
    // TODO(rqou): This is dumb.
    memcpy(NGL_BUFFER_DATA(program), code_buffer, code_buffer_len);
    // TODO(rqou): Dealloc code_buffer???
    ngl_run_package((ngl_package *) NGL_BUFFER_DATA(program));
    // TODO(rqou): Error handling?
    // TODO(rqou): What to do here?
    while (1) {}
  } else {
    // Load Lua blob
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Register builtins
    lua_register(L, "set_motor_old", lua_set_motor_old);
    lua_register(L, "get_remote", lua_get_sensor);
    lua_register_all(L);

    // Load the code blob into the Lua state
    const char *read_from_code_buffer(lua_State *L, void *data, size_t *size) {
      int *data_int = (int*)data;
      if (*data_int) {
        return NULL;
      } else {
        *data_int = 1;
        *size = code_buffer_len;
        return (const char *)code_buffer;
      }
    }
    int reader_state = 0;
    #ifndef TEST_STATIC_LUA
      lua_load(L, read_from_code_buffer, &reader_state, "<tenshi>", "b");
    #else
      luaL_loadstring(L, TEST_STATIC_LUA);
    #endif
    // TODO(rqou): Error handling?
    lua_pcall(L, 0, LUA_MULTRET, 0);
    // TODO(rqou): What to do here?
    while (1) {}
  }
}

// TODO(rqou): Move this elsewhere
// TODO(rqou): This entire function is a hack
static portTASK_FUNCTION_PROTO(radioTask, pvParameters) {
  (void) pvParameters;

  // TODO(rqou): More intelligent interleaved
  uint32_t code_received_to = 0;
  uint32_t code_received_len = 0;
  int got_a_packet = 0;
  int should_harass = 0;
  const uint32_t CHUNK_SIZE = 64;
  // TODO(rqou): Hack
  uint8_t txbuf[64];
  // TODO(rqou): Ugly uglly
  uint64_t host_addr = 0;
  // TODO(rqou): Remove this (why does this crash anyways?)
  int started_angelic = 0;

  while (1) {
    // TODO(rqou): Size is hardcoded. Yuck.
    uint8_t buf[256];
    int ret;
    size_t len;

    if (!got_a_packet && should_harass) {
      // Harass host
      xbee_api_packet *packetOut = (xbee_api_packet *)(txbuf);
      packetOut->xbee_api_magic = XBEE_MAGIC;
      int payloadLen = sizeof(xbee_tx64_header) + sizeof(tenshi_bulk_chunkreq);
      packetOut->length = __REV16(payloadLen);
      packetOut->payload.tx64.xbee_api_type = XBEE_API_TYPE_TX64;
      packetOut->payload.tx64.frameId = 0;
      packetOut->payload.tx64.xbee_dest_addr = host_addr;
      packetOut->payload.tx64.options = 0;
      tenshi_bulk_chunkreq *bulk_chunkreq =
        (tenshi_bulk_chunkreq *)(packetOut->payload.tx64.data);
      bulk_chunkreq->ident = TENSHI_NAIVE_BULK_CHUNKREQ_IDENT;
      bulk_chunkreq->stream_id = 0;
      bulk_chunkreq->start_addr = code_received_to;
      if (code_received_to + CHUNK_SIZE > code_received_len) {
        bulk_chunkreq->end_addr = code_received_len;
      } else {
        bulk_chunkreq->end_addr = code_received_to + CHUNK_SIZE;
      }
      xbee_fill_checksum(packetOut);

      // TODO(rqou): Asynchronous?
      // TODO(rqou): Abstract away the +4 properly
      // TODO(rqou): Error handling
      void *txn = uart_serial_send_data(radio_driver, txbuf, payloadLen + 4);
      while ((uart_serial_send_status(radio_driver, txn) !=
          UART_SERIAL_SEND_DONE) &&
          (uart_serial_send_status(radio_driver, txn) !=
            UART_SERIAL_SEND_ERROR)) {}
      uart_serial_send_finish(radio_driver, txn);
    }
    got_a_packet = 0;

    len = sizeof(buf);
    ret = uart_serial_receive_packet(radio_driver, buf, &len, 0);
    if (ret) {
      // TODO(rqou): Proper timer, why the f*ck do I need to copy this here?
      vTaskDelay(20 / portTICK_RATE_MS);
      continue;
    }
    xbee_api_packet *packetIn = (xbee_api_packet *)buf;
    if (!xbee_verify_checksum(packetIn)) {
      // TODO(rqou): Proper timer, why the f*ck do I need to copy this here?
      vTaskDelay(20 / portTICK_RATE_MS);
      continue;
    }
    if (packetIn->payload.xbee_api_type != XBEE_API_TYPE_RX64) {
      // TODO(rqou): Proper timer, why the f*ck do I need to copy this here?
      vTaskDelay(20 / portTICK_RATE_MS);
      continue;
    }
    // ident byte for PiEMOS framing
    switch (packetIn->payload.rx64.data[0]) {
    case PIER_INCOMINGDATA_IDENT:
      {
        pier_incomingdata *incomingData =
          (pier_incomingdata *)(packetIn->payload.rx64.data);
        // TODO(rqou): This code is terribly hardcoded.
        for (int i = 0; i < 7; i++) {
          PiEMOSAnalogVals[i] =
            (float)((int)incomingData->analog[i] - 127) / 127.0f * 100.0f;
        }
        for (int i = 0; i < 8; i++) {
          PiEMOSDigitalVals[i] = !!(incomingData->digital & (1 << i));
        }
      }
      break;

    // Naive bulk protocol
    case TENSHI_NAIVE_BULK_START_IDENT:
      {
        tenshi_bulk_start *bulk_start =
          (tenshi_bulk_start *)(packetIn->payload.rx64.data);
        // TODO(rqou): What happens if I already have one?
        // TODO(rqou): Stream ID?
        code_buffer = malloc(bulk_start->length);
        code_received_to = 0;
        code_buffer_len = code_received_len = bulk_start->length;
        got_a_packet = 1;
        should_harass = 1;

        // TODO(rqou): Refactor this logic
        host_addr = packetIn->payload.rx64.xbee_src_addr;
      }
      break;
    case TENSHI_NAIVE_BULK_CHUNK_IDENT:
      {
        tenshi_bulk_chunk *bulk_chunk =
          (tenshi_bulk_chunk *)(packetIn->payload.rx64.data);
        memcpy(code_buffer + bulk_chunk->start_addr, bulk_chunk->data,
          bulk_chunk->end_addr - bulk_chunk->start_addr);
        // TODO(rqou): Properly handle out-of-order
        code_received_to = bulk_chunk->end_addr;
        got_a_packet = 1;

        if (code_received_to == code_received_len) {
          xbee_api_packet *packetOut = (xbee_api_packet *)(txbuf);
          packetOut->xbee_api_magic = XBEE_MAGIC;
          int payloadLen = sizeof(xbee_tx64_header) + sizeof(tenshi_bulk_stop);
          packetOut->length = __REV16(payloadLen);
          packetOut->payload.tx64.xbee_api_type = XBEE_API_TYPE_TX64;
          packetOut->payload.tx64.frameId = 0;
          packetOut->payload.tx64.xbee_dest_addr =
            packetIn->payload.rx64.xbee_src_addr;
          packetOut->payload.tx64.options = 0;
          tenshi_bulk_stop *bulk_stop =
            (tenshi_bulk_stop *)(packetOut->payload.tx64.data);
          bulk_stop->ident = TENSHI_NAIVE_BULK_STOP_IDENT;
          bulk_stop->stream_id = 0;
          xbee_fill_checksum(packetOut);

          should_harass = 0;

          // TODO(rqou): Asynchronous?
          // TODO(rqou): Abstract away the +4 properly
          // TODO(rqou): Error handling
          void *txn =
            uart_serial_send_data(radio_driver, txbuf, payloadLen + 4);
          while ((uart_serial_send_status(radio_driver, txn) !=
              UART_SERIAL_SEND_DONE) &&
              (uart_serial_send_status(radio_driver, txn) !=
                UART_SERIAL_SEND_ERROR)) {}
          uart_serial_send_finish(radio_driver, txn);

          if (!started_angelic) {
            started_angelic = 1;
            xTaskCreate(angelicTask, "Angelic", 2048, NULL,
              tskIDLE_PRIORITY, NULL);
          }
        }
      }
      break;

    default:
      // TODO(rqou): Report error or something?
      break;
    }

    // TODO(rqou): Proper timer
    vTaskDelay(20 / portTICK_RATE_MS);
  }
}

int main(int argc, char **argv) {
  // Not useful
  (void) argc;
  (void) argv;

  init_malloc_lock();

  led_driver_init();
  button_driver_init();

  // Setup I2C
  i2c1_init();

  // Setup SmartSensors
  smartsensor_init();

  // Setup radio
  const int oldRadio = 0;
  if (oldRadio) {
    radio_driver_init();
    xTaskCreate(radioTask, "Radio", 2048, NULL, tskIDLE_PRIORITY, NULL);
  } else {
    radioInit();
  }

  runtimeInit();

  vTaskStartScheduler();
}
