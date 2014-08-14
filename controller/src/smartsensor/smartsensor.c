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
#include "inc/smartsensor/enumeration.h"
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

struct BandwidthAllocation {
  int16_t sensorMapping[SS_NUM_SAMPLES][SS_NUM_FRAMES];
};

// Linked list
struct SensorCallback {
  void(*func)(uint16_t, SSState *sensor);
  struct SensorCallback *next;
};



// Initialize with -1
// Specifies the index of the sensor in sensorArr assigned to a given sample
// and frame (both zero indexed).
volatile struct BandwidthAllocation sensorMapping[SS_BUS_COUNT] =
  {[0 ... SS_BUS_COUNT-1] = {.sensorMapping =
                             {[0 ... SS_NUM_SAMPLES-1] =
                              {[0 ... SS_NUM_FRAMES-1] = -1}}}};
volatile SS_BUS_STATE busState[SS_BUS_COUNT] =
                        {[0 ... SS_BUS_COUNT-1] = SS_BUS_ENUMERATION};

EventGroupHandle_t busStateEvent = NULL;
int allActive = 0;

// Linked list of callback functions
struct SensorCallback *sensorCallbacks = NULL;
xSemaphoreHandle sensorCallbackLock = NULL;
QueueHandle_t sensorUpdateQueue = NULL;  // Private queue



portTASK_FUNCTION_PROTO(smartSensorUpdateTask, pvParameters);




void smartsensor_init() {
  // Init busses
  smartsensor1_init();
  smartsensor2_init();
  smartsensor3_init();
  smartsensor4_init();

  busStateEvent = xEventGroupCreate();

  sensorCallbackLock = xSemaphoreCreateBinary();
  xSemaphoreGive(sensorCallbackLock);

  sensorUpdateQueue = xQueueCreate(100, sizeof(uint16_t));

  // Init sensor array
  sensorArrLock = xSemaphoreCreateBinary();

  numSensorsAlloc = numSensors = 0;
  sensorArr = pvPortMalloc(numSensorsAlloc*sizeof(SSState*));

  xSemaphoreGive(sensorArrLock);

  // Start tasks
  xTaskCreate(smartSensorUpdateTask, (const char *)"SensorTX", 200, NULL,
    tskIDLE_PRIORITY, NULL);
  for (int i = 0; i < SS_BUS_COUNT; i++) {
    busState[i] = SS_BUS_ENUMERATION;
    xTaskCreate(smartSensorTX, (const char *)"SensorTX", 1024, (void*)i,
      tskIDLE_PRIORITY, NULL);
  }
  for (int i = 0; i < SS_BUS_COUNT; i++) {
    xTaskCreate(smartSensorRX, (const char *)"SensorRX", 200, (void*)i,
      tskIDLE_PRIORITY, NULL);
  }
}
int ssIsActive() {
  return allActive;
}
void ssBlockUntilActive() {
  if (!allActive) {
    EventBits_t waitBits = ~((~0) << (SS_BUS_COUNT));
    xEventGroupSync(busStateEvent, 0, waitBits, portMAX_DELAY);
  }
}
void registerSensorUpdateCallback(void(*func)(uint16_t i, SSState *sensor)) {
  if (xSemaphoreTake(sensorCallbackLock, SEMAPHORE_WAIT_TIME) == pdTRUE) {
    struct SensorCallback *callback =
                                  pvPortMalloc(sizeof(struct SensorCallback));
    callback->func = func,
    callback->next = NULL;

    if (sensorCallbacks == NULL) {
      sensorCallbacks = callback;
    } else {
      struct SensorCallback *last = sensorCallbacks;
      while (last->next != NULL) {
        last = last->next;
      }
      last->next = callback;
    }
    xSemaphoreGive(sensorCallbackLock);
  }
}



portTASK_FUNCTION_PROTO(smartSensorTX, pvParameters) {
  (void) pvParameters;

  int busNum = (int)pvParameters;
  uart_serial_module *bus = ssBusses[busNum];
  volatile struct BandwidthAllocation *busSensorMapping =
                                                       &sensorMapping[busNum];

  vTaskDelay(200 / portTICK_RATE_MS);  // Wait for smart sensors to boot.

  while (busState[busNum] == SS_BUS_ENUMERATION) {
    led_driver_set_mode(PATTERN_ENUMERATING);

    KnownIDs enumIDs = {
      .arr = pvPortMalloc(SS_MAX_SENSORS_PER_BUS * sizeof(SSState*)),
      .len = 0,
      .maxLen = SS_MAX_SENSORS_PER_BUS,
    };

    if (enumerateSensors(&enumIDs, bus, busNum)) {
    } else {
      enumIDs.len = 0;  // Ignore sensors from failed attempt.
    }

    size_t index = ss_add_sensors(&enumIDs);

    // Bandwidth allocation and assignment
    // TODO(cduck): Better bandwidth allocation (only allocates first 6)
    for (int i = 0; i < SS_NUM_FRAMES && i < enumIDs.len; ++i) {
      SSState *sensor = enumIDs.arr[i];

      ss_update_descriptor(sensor);

      // Assign bandwidth to sensor
      uint8_t data[SMART_ID_LEN+2];
      data[SMART_ID_LEN] = 0xFF;
      data[SMART_ID_LEN+1] = (uint8_t)(i+SS_FIRST_FRAME);
      memcpy(data, sensor->id, SMART_ID_LEN);
      ss_send_maintenance(bus, 0xD0, data, SMART_ID_LEN+2);
      ss_select_delay();

      // Store bandwidth assignment
      for (int s = 0; s < SS_NUM_SAMPLES; ++s) {
        busSensorMapping->sensorMapping[s][i] = index+i;
      }

      ss_select_delay();
    }

    vPortFree(enumIDs.arr);

    busState[busNum] = SS_BUS_ACTIVE;
  }

  while (busState[busNum] == SS_BUS_MAINTAINANCE) {
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

    busState[busNum] = SS_BUS_ACTIVE;
  }

  // Waiting for all threads to be ready to enter active state
  EventBits_t setBit = 1 << busNum;
  EventBits_t waitBits = ~((~0) << (SS_BUS_COUNT));
  xEventGroupSync(busStateEvent, setBit, waitBits, portMAX_DELAY);

  // Only one thread needs to do this
  if (busNum == 0) {
    // Set all active flag (atomic)
    allActive = 1;

    // Display number of sensors
    led_driver_set_mode(PATTERN_JUST_RED);
    led_driver_set_fixed(numSensors, 0b111);
  }


  while (busState[busNum] == SS_BUS_ACTIVE) {
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

    // TODO(cduck): Better timing
    while (1) {
      // Send entire sample (about 1ms for 100 bytes)

      uint8_t isOutgoing = 0;
      int16_t sensorIndex =
        busSensorMapping->sensorMapping[sampleNumber%SS_NUM_SAMPLES]
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

      vTaskDelay(1 / portTICK_RATE_MS);


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

  while (1) {}
}

portTASK_FUNCTION_PROTO(smartSensorRX, pvParameters) {
  (void) pvParameters;

  int busNum = (int)pvParameters;
  uart_serial_module *bus = ssBusses[busNum];
  volatile struct BandwidthAllocation *busSensorMapping =
                                                       &sensorMapping[busNum];

  // Recieve
  size_t recLen = 0;

  while (1) {
    while (busState[busNum] != SS_BUS_ACTIVE) {}

    while (busState[busNum] == SS_BUS_ACTIVE) {
      // Recieve the response to the packet
      // 1 means wait for packet
      uint8_t *data = uart_serial_receive_packet(bus, &recLen, 0);

      if (busState[busNum] != SS_BUS_ACTIVE) break;

      if (data) {
        uint8_t prefixLen = 3;
        if (recLen > prefixLen) {
          uint8_t sampleNumber = (data[1] >> 3) & 0b111;
          // Zero indexed
          uint8_t frameNumber = (data[1] & 0b111) - SS_FIRST_FRAME;
          uint8_t inband = data[1] >> 7;
          uint8_t decodeLen = recLen-prefixLen-1;
          uint8_t *data_decode = pvPortMalloc(decodeLen);
          cobs_decode(data_decode, data+prefixLen, decodeLen+1);

          // led_driver_set_mode(PATTERN_JUST_RED);
          // led_driver_set_fixed(sampleNumber, 0b111);

          if (frameNumber < SS_NUM_FRAMES && sampleNumber < SS_NUM_SAMPLES) {
            int16_t sensorIndex =
              busSensorMapping->sensorMapping[sampleNumber%SS_NUM_SAMPLES]
                           [frameNumber];
            if (sensorIndex >= 0 && numSensors > sensorIndex) {
              SSState *sensor = sensorArr[sensorIndex];
              ss_recieved_data_for_sensor(sensor, data_decode, decodeLen,
                inband);
              xQueueSend(sensorUpdateQueue, &sensorIndex, 0);
            }
          }

          if (data_decode) vPortFree(data_decode);
        }
        vPortFree(data);
      }
    }
  }
}


portTASK_FUNCTION_PROTO(smartSensorUpdateTask, pvParameters) {
  (void) pvParameters;

  uint16_t index;
  while (1) {
    if (xQueueReceive(sensorUpdateQueue, &index, portMAX_DELAY)) {
      struct SensorCallback *callback = sensorCallbacks;
      while (callback != NULL) {
        callback->func(index, sensorArr[index]);
        callback = callback->next;
      }
    }
  }
}

