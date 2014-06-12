#include <stdio.h>
#include <string.h>

/*
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>
*/

#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/smartsensor/cobs.h"

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

/*
#include "legacy_piemos_framing.h"   // NOLINT(build/include)
#include "ngl_types.h"   // NOLINT(build/include)
*/


typedef enum {
  SS_BUS_INACTIVE,
  SS_BUS_MAINTAINANCE,
  SS_BUS_ENUMERATION,
  SS_BUS_ACTIVE
} SS_BUS_STATE;




portTASK_FUNCTION_PROTO(smartSensorTX, pvParameters) {
  (void) pvParameters;

  // TODO(cduck): Don't start with active state.  Write code for other states.
  SS_BUS_STATE busState = SS_BUS_ACTIVE;


  while (1) {
    while (busState == SS_BUS_ENUMERATION) {
      break;
    }

    if (busState == SS_BUS_ACTIVE) {
      // At the beginning of each subchunk, the master sends the following
      // header:
      // |----------------------------------------
      // | 0x00 | has payload  sample#  subchunk#
      // |----------------------------------------
      //   ---------------------------------------------|
      //   | <in-band signalling>  <length> | <payload> |
      //   ---------------------------------------------|

      // size_t len = 16*6+4;
      // Bit format:
      // {0, 0b0xyyzzzz, 0bslllllll, p, p, p, p, p, p, p, p, p, p, p, p, p}
      // x=has payload, y=sample # (mod 4 (was 8)) (0-indexed),
      // z=subchunk # (1-indexed)
      // s=in-band signalling, l=length of payload, p=payload (only when s=1)
      uint8_t data_len = 100;
      uint8_t data[] =
         {0, 0b00000001, 0b00000101, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0b00000010, 0b00000011, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0b00000011, 0b00000011, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0b00000100, 0b00000011, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0b00000101, 0b00000011, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0b00000110, 0b00000011, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0  // Timing bytes
         };
      uint8_t d_len = 7 +8;
      uint8_t d[] = {0, 1, 2, 3, 4, 5, 0x42, 7, 0x01, 0x10, 0x00, 0x44, 0x88,
                     0xCC, 0xFF};
      uint8_t d2_len = 7;
      uint8_t d2[] = {0x01, 0x10, 0x00, 0x44, 0x88, 0xCC, 0xFF};
      SSState sensor1 = {
        .id = {0, 1, 2, 3, 4, 5, 0x42, 7},
        .busNum = 2};
      SSState sensor2 = {
        .id = {0, 1, 2, 3, 4, 5, 0x42, 7},
        .busNum = 0};

      uint8_t d3_len = 6;
      uint8_t d3[] = {0, 0xFF, d3_len, 9, 1, 0x10};
      // uint8_t d3[] = {0, 6, 2, 1, 2};

      // TODO(cduck): Better timing
      while (1) {
        // Send entire sample (about 1ms for 100 bytes)

        uint8_t button = button_driver_get_button_state(0);

        // Ping pong packet
        sensor1.id[2] ^= 5;
        sensor2.id[2] ^= 5;
        d[2] ^= 5;
        data[3] = button ? 2 : 1;
        // ss_all_send_maintenance(0xFE, d, d_len);
        // ss_send_maintenance(smartsensor_1, 0xFE, d, d_len);
        // ss_send_maintenance(smartsensor_2, 0xFE, d, d_len);
        ss_send_ping_pong(sensor1, d2, d2_len);
        // ss_send_ping_pong(sensor2, d, d_len);
        ss_uart_serial_send_and_finish_data(smartsensor_4, d3, d3_len);
        ss_uart_serial_send_and_finish_data(smartsensor_1, data, data_len);

        // Increment sample number in bits 4-6 (mod 8)
        // assuming has payload=0
        for (uint8_t i = 0; i < 6; i++) {
          data[1+i*16] = (data[1+i*16] + 0b1000) & 0b00111111;
        }

        // vTaskDelay(200 / portTICK_RATE_MS);  // /////////////////
      }
    }
  }
}

portTASK_FUNCTION_PROTO(smartSensorRX, pvParameters) {
  (void) pvParameters;

  // TODO(cduck): Don't start with active state.  Write code for other states.
  SS_BUS_STATE busState = SS_BUS_ACTIVE;

  while (1) {
    while (busState == SS_BUS_ENUMERATION) {
      break;
    }

    // if (busState == SS_BUS_ACTIVE) {
    // uint8_t a = 0xaa;
    // uint8_t b = 0xbb;
    while (1) {
      size_t len = 0;
      // 1 means wait for packet
      uint8_t *data = uart_serial_receive_packet(smartsensor_1, &len, 1);

      if (!data)continue;
      if (len <= 2) {
        vPortFree(data);
        continue;
      }

      uint8_t button = button_driver_get_button_state(0);

      uint8_t *data_decode = pvPortMalloc(len-3);
      cobs_decode(data_decode, data+2, len-2);
      led_driver_set_mode(PATTERN_JUST_RED);
      led_driver_set_fixed(data_decode[0]&3 | (button << 2), 0b111);

      if (data_decode) vPortFree(data_decode);
      if (data) vPortFree(data);
    }
  }
}

