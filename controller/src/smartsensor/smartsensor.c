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



// Initialize with -1
// Specifies the index of the sensor in sensorArr assigned to a given sample
// and frame (both zero indexed).
int16_t sensorMapping[SS_NUM_SAMPLES][SS_NUM_FRAMES] = {{-1}};
SS_BUS_STATE busState = SS_BUS_ENUMERATION;



void smartsensor_init() {
  // Init busses
  smartsensor1_init();
  smartsensor2_init();
  smartsensor3_init();
  smartsensor4_init();

  // Init sensor array
  sensorArrLock = xSemaphoreCreateBinary();

  numSensorsAlloc = numSensors = 0;
  sensorArr = pvPortMalloc(numSensorsAlloc*sizeof(SSState*));

  xSemaphoreGive(sensorArrLock);

  // Start tasks
  xTaskCreate(smartSensorTX, (const char *)"SensorTX", 2048, NULL,
    tskIDLE_PRIORITY, NULL);
  xTaskCreate(smartSensorRX, (const char *)"SensorRX", 2048, NULL,
    tskIDLE_PRIORITY, NULL);
}




portTASK_FUNCTION_PROTO(smartSensorTX, pvParameters) {
  (void) pvParameters;

  // TODO(cduck): Get bus num from pvParameters.
  uint8_t busNum = 0;
  uart_serial_module *bus = ssBusses[busNum];
  while (1) {
    vTaskDelay(200 / portTICK_RATE_MS);  // Wait for smart sensors to boot.

    while (busState == SS_BUS_ENUMERATION) {
      uint8_t id[] = {0, 0, 0, 0, 0, 0, 0, 0};
      static uint8_t maskAll[] =
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      static uint8_t maskNone[] = {0, 0, 0, 0, 0, 0, 0, 0};
      uint8_t unsel = 0;
      uint8_t unselOld = 0;
      uint8_t selectDelay = 1;
      size_t numSensorFound = 0;

      ss_send_enum_enter(bus);  // Enter enumeration
      vTaskDelay(selectDelay / portTICK_RATE_MS);
      unsel = ss_recieve_enum_any_unselected(bus);

      ss_send_enum_select(bus, id, maskNone);  // Select all sensors
      vTaskDelay(selectDelay / portTICK_RATE_MS);
      unselOld = unsel;
      unsel = ss_recieve_enum_any_unselected(bus);

      if (!unsel && unselOld) {
        // There are smart sensors on the bus
        for (int i = 0; i < 255; i++) {
          id[SMART_ID_LEN-1] = (uint8_t)i;

          ss_send_enum_unselect(bus, id, maskAll);  // Unselect one sensor
          vTaskDelay(selectDelay / portTICK_RATE_MS);
          unselOld = unsel;
          unsel = ss_recieve_enum_any_unselected(bus);
          if (unsel && !unselOld) {
            // Found a sensor with id of i
            size_t index = ss_add_new_sensor(id, busNum);

            // //////////////////////////////////////////////////
            // TODO(cduck): Better bandwidth allocation (only allocates first 6)
            if (numSensorFound < SS_NUM_FRAMES) {
              for (int i = 0; i < SS_NUM_SAMPLES; ++i) {
                sensorMapping[i][numSensorFound] = index;
              }
            }

            ++numSensorFound;
          }

          ss_send_enum_select(bus, id, maskNone);  // Select all sensors
          vTaskDelay(selectDelay / portTICK_RATE_MS);
          unselOld = unsel;
          unsel = ss_recieve_enum_any_unselected(bus);
        }
      } else {
        // There are no smart sensor on the bus
        while (1) {
          led_driver_set_mode(PATTERN_JUST_RED);
          led_driver_set_fixed(0b100, 0b111);
        }
      }

      ss_send_enum_exit(bus);
      vTaskDelay(selectDelay / portTICK_RATE_MS);

      led_driver_set_mode(PATTERN_JUST_RED);
      led_driver_set_fixed(0b111, 0b111);

      busState = SS_BUS_MAINTAINANCE;
    }

    vTaskDelay(50 / portTICK_RATE_MS);  // ///////

    while (busState == SS_BUS_MAINTAINANCE) {
      uint8_t d1_len = 7;
      uint8_t d1[] = {0x11, 0x22, 0x33, 0x44, 0x88, 0xCC, 0xFF};
      uint8_t d2_len = 100;
      uint8_t d2[251];
      for (int i = 0; i < 251; ++i) {
        d2[i] = i;
      }
      for (int i = 0; i < numSensors; ++i) {
        ss_send_ping_pong(sensorArr[i], d1, d1_len);
        vTaskDelay(2 / portTICK_RATE_MS);
      }

      busState = SS_BUS_ACTIVE;
    }

    while (busState == SS_BUS_ACTIVE) {
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

      unsigned int sampleNumber = 0;
      uint8_t frameNumber = SS_FIRST_FRAME;
      unsigned int sampleNumberLast = sampleNumber;
      uint8_t frameNumberLast = frameNumber;

      // Recieve
      size_t recLen = 0;

      transmit_allocations allocs = {.txn = NULL, .data = NULL};
      transmit_allocations allocsOld = {.txn = NULL, .data = NULL};

      // Clear missed packets
      // while (uart_serial_receive_packet(bus, &recLen, 0)) {}

      // TODO(cduck): Better timing
      while (1) {
        // Send entire sample (about 1ms for 100 bytes)

        // Clear missed packets
        // while (uart_serial_receive_packet(bus, &recLen, 0)) {}

        uint8_t isOutgoing = 0;
        int16_t sensorIndex =
          sensorMapping[sampleNumber%SS_NUM_SAMPLES]
                       [frameNumber-SS_FIRST_FRAME];
        SSState *sensor;
        if (sensorIndex >= 0 && numSensors > sensorIndex) {
          uint8_t button = button_driver_get_button_state(sensorIndex%2);
          sensor = sensorArr[sensorIndex];
          if (xSemaphoreTake(sensor->outLock, SENSOR_WAIT_TIME) == pdTRUE) {
            if (checkOutgoingBytes(sensor, 1)) {
              sensor->outgoingBytes[0] ^= ((0xFF*(!!button)) << 1);
              isOutgoing = 1;
              allocs = ss_send_active(bus, 0, sampleNumber, frameNumber,
                sensor->outgoingBytes, sensor->outgoingLen);
            } else {
              allocs = ss_send_active(bus, 0, sampleNumber, frameNumber,
                NULL, 0);
            }

            // Wait until the previous packet sent
            ss_wait_until_done(bus, allocs);
            // Clean up the after sending the previous packet
            ss_send_finish(bus, allocs);

            xSemaphoreGive(sensor->outLock);
          }
        }

        // //////////////////////Send
        // Queue the current packet for sending
        // Send active frame (non-blocking)
        /* if (isOutgoing) {

          xSemaphoreGive(sensor->outLock);
        } else {
          allocs = ss_send_active(bus, 0, sampleNumber, frameNumber,
            NULL, 0);
        }
        if (allocs.txn) {
          // Wait until the previous packet sent
          ss_wait_until_done(bus, allocs);
          // Clean up the after sending the previous packet
          ss_send_finish(bus, allocs);
        }
        allocsOld = allocs;*/

        vTaskDelay(1 / portTICK_RATE_MS);
/*
        // Recieve the response to the packet
        {
          // 1 means wait for packet
          uint8_t *data = uart_serial_receive_packet(bus, &recLen, 0);

          if (data) {
            if (recLen > 2) {
              uint8_t prefixLen = 3;
              uint8_t decodeLen = recLen-prefixLen-1;
              uint8_t *data_decode = pvPortMalloc(decodeLen);
              uint8_t freeData = 1;
              cobs_decode(data_decode, data+prefixLen, decodeLen+1);

              int16_t sensorIndex =
                sensorMapping[sampleNumber%SS_NUM_SAMPLES]
                             [frameNumber-SS_FIRST_FRAME];
              if (sensorIndex >= 0 && numSensors > sensorIndex) {
                SSState *sensor = sensorArr[sensorIndex];
                ss_recieved_data_for_sensor(sensor, data_decode, decodeLen, 0);
              }

              // Only free if not assigned to a sensor.
              if (data_decode && freeData) vPortFree(data_decode);
            }
            vPortFree(data);
          }
        }
*/

        // Update sample and frame numbers
        frameNumberLast = frameNumber;
        sampleNumberLast = sampleNumber;

        ++frameNumber;
        if (frameNumber >= SS_NUM_FRAMES+SS_FIRST_FRAME) {
          frameNumber = SS_FIRST_FRAME;
          ++sampleNumber;
        }
      }
    }
  }
}

portTASK_FUNCTION_PROTO(smartSensorRX, pvParameters) {
  (void) pvParameters;

  // TODO(cduck): Get bus num from pvParameters.
  uint8_t busNum = 0;
  uart_serial_module *bus = ssBusses[busNum];

  // Recieve
  size_t recLen = 0;

  while (1) {
    while (busState != SS_BUS_ACTIVE) {}

    while (busState == SS_BUS_ACTIVE) {
      // Recieve the response to the packet
      // 1 means wait for packet
      uint8_t *data = uart_serial_receive_packet(bus, &recLen, 0);

      if (busState != SS_BUS_ACTIVE) break;

      if (data) {
        uint8_t prefixLen = 3;
        if (recLen > prefixLen) {
          uint8_t sampleNumber = (data[1] >> 3) & 0b111;
          uint8_t frameNumber0 = data[1] & 0b111;  // Zero indexed
          uint8_t inband = data[1] >> 7;
          uint8_t decodeLen = recLen-prefixLen-1;
          uint8_t *data_decode = pvPortMalloc(decodeLen);
          cobs_decode(data_decode, data+prefixLen, decodeLen+1);

          led_driver_set_mode(PATTERN_JUST_RED);
          led_driver_set_fixed(sampleNumber, 0b111);

          int16_t sensorIndex =
            sensorMapping[sampleNumber%SS_NUM_SAMPLES]
                         [frameNumber0];
          if (sensorIndex >= 0 && numSensors > sensorIndex) {
            SSState *sensor = sensorArr[sensorIndex];
            ss_recieved_data_for_sensor(sensor, data_decode, decodeLen, inband);
          }

          if (data_decode) vPortFree(data_decode);
        }
        vPortFree(data);
      }
    }
  }
}

