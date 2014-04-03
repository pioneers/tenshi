#include <stdio.h>
#include <string.h>

#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>
#include "inc/FreeRTOS.h"
#include "inc/button_driver.h"
#include "inc/driver_glue.h"
#include "inc/i2c_master.h"
#include "inc/led_driver.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"
#include "inc/core_cm4.h"
#include "inc/core_cmInstr.h"
#include "inc/task.h"
#include "inc/xbee_framing.h"

#include "legacy_piemos_framing.h"   // NOLINT(build/include)

uint8_t *code_buffer;
uint32_t code_buffer_len;

static portTASK_FUNCTION_PROTO(angelicTask, pvParameters) {
  ngl_buffer *program = ngl_buffer_alloc(code_buffer_len);
  // TODO(rqou): This is dumb.
  memcpy(NGL_BUFFER_DATA(program), code_buffer, code_buffer_len);
  // TODO(rqou): Dealloc code_buffer???
  ngl_run_package((ngl_package *) NGL_BUFFER_DATA(program));
  // TODO(rqou): Error handling?
}

// TODO(rqou): Move this elsewhere
// TODO(rqou): This entire function is a hack
static portTASK_FUNCTION_PROTO(radioTask, pvParameters) {
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

  while (1) {
    uint8_t *buf;
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

    buf = uart_serial_receive_packet(radio_driver, &len, 0);
    if (!buf) {
      // TODO(rqou): Proper timer, why the f*ck do I need to copy this here?
      vTaskDelay(20 / portTICK_RATE_MS);
      continue;
    }
    if (!xbee_verify_checksum(buf)) {
      vPortFree(buf);
      // TODO(rqou): Proper timer, why the f*ck do I need to copy this here?
      vTaskDelay(20 / portTICK_RATE_MS);
      continue;
    }
    xbee_api_packet *packetIn = (xbee_api_packet *)buf;
    if (packetIn->payload.xbee_api_type != XBEE_API_TYPE_RX64) {
      vPortFree(buf);
      // TODO(rqou): Proper timer, why the f*ck do I need to copy this here?
      vTaskDelay(20 / portTICK_RATE_MS);
      continue;
    }
    // ident byte for PiEMOS framing
    switch (packetIn->payload.rx64.data[0]) {
    case PIER_INCOMINGDATA_IDENT:
      // TODO(rqou): do something useful!
      break;

    // Naive bulk protocol
    case TENSHI_NAIVE_BULK_START_IDENT:
      {
        tenshi_bulk_start *bulk_start =
          (tenshi_bulk_start *)(packetIn->payload.rx64.data);
        // TODO(rqou): What happens if I already have one?
        // TODO(rqou): Stream ID?
        code_buffer = pvPortMalloc(bulk_start->length);
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
          void *txn = uart_serial_send_data(radio_driver, buf, payloadLen + 4);
          while ((uart_serial_send_status(radio_driver, txn) !=
              UART_SERIAL_SEND_DONE) &&
              (uart_serial_send_status(radio_driver, txn) !=
                UART_SERIAL_SEND_ERROR)) {}
          uart_serial_send_finish(radio_driver, txn);

          xTaskCreate(
            angelicTask, "Angelic", 256, NULL, tskIDLE_PRIORITY, NULL);
        }
      }
      break;

    default:
      // TODO(rqou): Report error or something?
      break;
    }

    vPortFree(buf);

    // TODO(rqou): Proper timer
    vTaskDelay(20 / portTICK_RATE_MS);
  }
}

int main(int argc, char **argv) {
  led_driver_init();
  button_driver_init();

  // debug_uart_setup();

  // Setup I2C
  i2c1_init();

  // Setup SmartSensors
  smartsensor1_init();
  smartsensor2_init();
  smartsensor3_init();
  smartsensor4_init();

  // Setup radio
  radio_driver_init();

  xTaskCreate(radioTask, "Radio", 256, NULL, tskIDLE_PRIORITY, NULL);
  vTaskStartScheduler();
}
