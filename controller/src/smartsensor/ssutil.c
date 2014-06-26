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

#include "inc/smartsensor/ssutil.h"
#include "inc/smartsensor/cobs.h"

#include "inc/driver_glue.h"
#include "inc/uart_serial_driver.h"


EventGroupHandle_t ssBusEventGroup;

size_t numSensors = 0;
size_t numSensorsAlloc = 0;
SSState **sensorArr = NULL;
xSemaphoreHandle sensorArrLock;



// Functions to enable external code to set and read sensors
void ss_set_digital_value(int sensorIndex, uint8_t val) {
  ss_set_value(sensorIndex, &val, 1);
}
uint8_t ss_get_digital_value(int sensorIndex) {
  // If no value has been recieved yet default to 0.
  uint8_t result = 0;
  ss_get_value(sensorIndex, &result, 1);
  return result;
}

void ss_set_analog_value(int sensorIndex, unsigned int val) {
  #define val_len 4
  uint8_t data[val_len] = {val, val>>8, val>>16, val>>24};
  ss_set_value(sensorIndex, data, val_len);
  #undef val_len
}
unsigned int ss_get_analog_value(int sensorIndex) {
  #define val_len 4
  // If no value has been recieved yet default to 0.
  uint8_t data[val_len] = {0};
  ss_get_value(sensorIndex, data, val_len);

  uint32_t result = 0;
  for (uint8_t i = 0; i < val_len; ++i) {
    result |= ((uint32_t)data[i]) << (i*8);
  }
  return (unsigned int)result;
  #undef val_len
}

void ss_set_motor_value(int sensorIndex, uint8_t mode, double speed) {
  uint8_t data[5];
  data[0] = mode;
  int32_t speed32 = (int32_t)(speed * 65536.f);
  data[1] = speed32;
  data[2] = speed32 >> 8;
  data[3] = speed32 >> 16;
  data[4] = speed32 >> 24;
  ss_set_value(sensorIndex, data, 5);
}

void ss_set_value(int sensorIndex, uint8_t *data, size_t len) {
  if (numSensors > sensorIndex) {
    SSState *sensor = sensorArr[sensorIndex];
    if (xSemaphoreTake(sensor->outLock, SENSOR_WAIT_TIME) == pdTRUE) {
      allocOutgoingBytes(sensor, len);
      memcpy(sensor->outgoingBytes, data, len);

      xSemaphoreGive(sensor->outLock);
    }
  }
}
int ss_get_value(int sensorIndex, uint8_t *data, size_t len) {
  int lenDiff = 0;
  if (numSensors > sensorIndex) {
    SSState *sensor = sensorArr[sensorIndex];
    if (xSemaphoreTake(sensor->inLock, SENSOR_WAIT_TIME) == pdTRUE) {
      lenDiff = -((int)len);
      if (sensor->incomingBytes != NULL) {
        lenDiff += sensor->incomingLen;
        memcpy(data, sensor->incomingBytes,
          len < sensor->incomingLen ? len : sensor->incomingLen);
      }
      xSemaphoreGive(sensor->inLock);
    }
  }
  return lenDiff;
}



// Helper functions for smartsensor.c
SSState *ss_init_sensor(uint8_t id[SMART_ID_LEN], uint8_t busNum) {
  SSState *s = pvPortMalloc(sizeof(SSState));
  s->busNum = busNum;
  s->outLock = xSemaphoreCreateBinary();
  s->outgoingLen = 0;
  s->outgoingBytes = NULL;
  s->inLock = xSemaphoreCreateBinary();
  s->incomingLen = 0;
  s->incomingBytes = NULL;
  memcpy(s->id, id, SMART_ID_LEN);

  xSemaphoreGive(s->outLock);
  xSemaphoreGive(s->inLock);

  return s;
}
size_t ss_add_new_sensor(uint8_t id[SMART_ID_LEN], uint8_t busNum) {
  return ss_add_sensor(ss_init_sensor(id, busNum));
}
size_t ss_add_sensor(SSState *sensor) {
  if (xSemaphoreTake(sensorArrLock, SENSOR_WAIT_TIME) != pdTRUE) return -1;

  const int numAllocAtOnce = 10;
  if (numSensorsAlloc <= numSensors) {
    SSState **newPtr = pvPortMalloc((numSensorsAlloc+numAllocAtOnce) *
                                   sizeof(SSState*));
    memcpy(newPtr, sensorArr, numSensorsAlloc*sizeof(SSState*));
    vPortFree(sensorArr);
    sensorArr = newPtr;
    numSensorsAlloc += numAllocAtOnce;
  }

  size_t index = numSensors;
  sensorArr[index] = sensor;
  ++numSensors;
  xSemaphoreGive(sensorArrLock);
  return index;
}
size_t ss_add_sensors(KnownIDs *sensors) {
  if (sensors->len <= 0) return -1;
  if (xSemaphoreTake(sensorArrLock, SENSOR_WAIT_TIME) != pdTRUE) return -1;

  if (numSensorsAlloc < numSensors+sensors->len) {
    SSState **newPtr = pvPortMalloc((numSensors+sensors->len) *
                                   sizeof(SSState*));
    memcpy(newPtr, sensorArr, numSensors*sizeof(SSState*));
    vPortFree(sensorArr);
    sensorArr = newPtr;
    numSensorsAlloc = numSensors+sensors->len;
  }

  size_t index = numSensors;
  memcpy(sensorArr+numSensors, sensors->arr, sensors->len*sizeof(SSState*));
  numSensors += sensors->len;
  xSemaphoreGive(sensorArrLock);
  return index;
}
void ss_recieved_data_for_sensor(SSState *s, uint8_t *data, size_t len,
  uint8_t inband) {
  // TODO(cduck): Handle in band signalling
  if (xSemaphoreTake(s->inLock, SENSOR_WAIT_TIME) == pdTRUE) {
    allocIncomingBytes(s, len);
    s->incomingLen = len;
    memcpy(s->incomingBytes, data, len);

    xSemaphoreGive(s->inLock);
  }
}
// Assuming the sensor is already locked
void allocIncomingBytes(SSState *sensor, uint8_t requiredLen) {
  if (sensor->incomingBytes == NULL) {
    sensor->incomingLen = requiredLen;
    sensor->incomingBytes = pvPortMalloc(requiredLen);
  }
  if (sensor->incomingLen < requiredLen) {
    vPortFree(sensor->incomingBytes);
    sensor->incomingLen = requiredLen;
    sensor->incomingBytes = pvPortMalloc(requiredLen);
  }
}
// Assuming the sensor is already locked
int checkOutgoingBytes(SSState *sensor, uint8_t requiredLen) {
  return (sensor->outgoingBytes != NULL) &&
         (sensor->outgoingLen >= requiredLen);
}
// Assuming the sensor is already locked
void allocOutgoingBytes(SSState *sensor, uint8_t requiredLen) {
  if (sensor->outgoingBytes == NULL) {
    sensor->outgoingLen = requiredLen;
    sensor->outgoingBytes = pvPortMalloc(requiredLen);
  }
  if (sensor->outgoingLen < requiredLen) {
    vPortFree(sensor->outgoingBytes);
    sensor->outgoingLen = requiredLen;
    sensor->outgoingBytes = pvPortMalloc(requiredLen);
  }
}
// Assuming the sensor is already locked
int checkIncomingBytes(SSState *sensor, uint8_t requiredLen) {
  return (sensor->incomingBytes != NULL) &&
         (sensor->incomingLen >= requiredLen);
}


// Helper functions for smartsensor.c
// TODO(cduck): Move this to uart_serial_driver.c and make it work
int ss_uart_serial_send_and_finish_data(uart_serial_module *module,
  const uint8_t *data, size_t len) {
  // TODO(rqou): Asynchronous?
  // TODO(rqou): Error handling
  void *txn = uart_serial_send_data(module, data, len);
  while ((uart_serial_send_status(module, txn) !=
      UART_SERIAL_SEND_DONE) &&
      (uart_serial_send_status(module, txn) !=
        UART_SERIAL_SEND_ERROR)) {}
  return uart_serial_send_finish(module, txn);
}
// TODO(cduck): Enable COMM3 after finished using it for debugging
int ss_all_uart_serial_send_and_finish_data(const uint8_t *data, size_t len) {
  // TODO(rqou): Asynchronous?
  // TODO(rqou): Error handling
  void *txn1 = uart_serial_send_data(smartsensor_1, data, len);
  void *txn2 = uart_serial_send_data(smartsensor_2, data, len);
  // void *txn3 = uart_serial_send_data(smartsensor_3, data, len);
  // void *txn4 = uart_serial_send_data(smartsensor_4, data, len);
  while (((uart_serial_send_status(smartsensor_1, txn1)
              != UART_SERIAL_SEND_DONE) &&
          (uart_serial_send_status(smartsensor_1, txn1)
              != UART_SERIAL_SEND_ERROR) ) ||
         ((uart_serial_send_status(smartsensor_2, txn2)
              != UART_SERIAL_SEND_DONE) &&
          (uart_serial_send_status(smartsensor_2, txn2)
              != UART_SERIAL_SEND_ERROR) )
         //      ||
         // ((uart_serial_send_status(smartsensor_3, txn3)
         //      != UART_SERIAL_SEND_DONE) &&
         //  (uart_serial_send_status(smartsensor_3, txn3)
         //      != UART_SERIAL_SEND_ERROR) ) ||
         // ((uart_serial_send_status(smartsensor_4, txn4)
         //     != UART_SERIAL_SEND_DONE) &&
         // (uart_serial_send_status(smartsensor_4, txn4)
         //     != UART_SERIAL_SEND_ERROR) )
         ) {}
  return uart_serial_send_finish(smartsensor_1, txn1) &
         uart_serial_send_finish(smartsensor_2, txn2);  // &
         // uart_serial_send_finish(smartsensor_3, txn3) &
         // uart_serial_send_finish(smartsensor_4, txn4);
}
int ss_send_maintenance(uart_serial_module *module, uint8_t type,
  const uint8_t *data, uint8_t len) {
  if (len > 255-4)return 0;

  uint8_t *data_cobs = (uint8_t*)pvPortMalloc(4+len);
  // Four extra for 0x00, type, len, and extra COBS byte.

  data_cobs[0] = 0x00;
  data_cobs[1] = type;
  data_cobs[2] = len+4;
  cobs_encode(data_cobs+3, data, len);

  int r = ss_uart_serial_send_and_finish_data(module, data_cobs, len+4);
  vPortFree(data_cobs);
  return r;
}
int ss_send_maintenance_to_sensor(SSState *sensor, uint8_t type,
  const uint8_t *data, uint8_t len) {
  uint8_t *buffer = pvPortMalloc(SMART_ID_LEN+len);
  memcpy(buffer, sensor->id, SMART_ID_LEN);
  memcpy(buffer+SMART_ID_LEN, data, len);
  int ret = ss_send_maintenance(ssBusses[sensor->busNum], type, buffer,
    SMART_ID_LEN+len);
  vPortFree(buffer);
  return ret;
}
int ss_all_send_maintenance(uint8_t type, const uint8_t *data, uint8_t len) {
  if (len > 255-4)return 0;

  uint8_t *data_cobs = (uint8_t*)pvPortMalloc(4+len);
  // Four extra for 0x00, type, len, and extra COBS byte.

  data_cobs[0] = 0x00;
  data_cobs[1] = type;
  data_cobs[2] = len+4;
  cobs_encode(data_cobs+3, data, len);

  int r = ss_all_uart_serial_send_and_finish_data(data_cobs, len+4);
  vPortFree(data_cobs);
  return r;
}
int ss_send_ping_pong(SSState *sensor, const uint8_t *data, uint8_t len) {
  if (len > 255-4-SMART_ID_LEN)return 0;

  uint8_t *temp = (uint8_t*)pvPortMalloc(len+SMART_ID_LEN);
  for (uint8_t i = 0; i < SMART_ID_LEN; i++) {
    temp[i] = sensor->id[i];
  }
  for (uint8_t i = 0; i < len; i++) {
    temp[SMART_ID_LEN+i] = data[i];
  }

  int r = ss_send_maintenance(ssBusses[sensor->busNum], 0xFE, temp,
    len+SMART_ID_LEN);
  vPortFree(temp);
  return r;
}

int ss_send_enum_enter(uart_serial_module *module) {
  #define ENUM_ENTER_LEN (MAGIC_SEQUENCE_LEN+4)
  static const uint8_t data[] = {0, 0xF0, ENUM_ENTER_LEN, MAGIC_SEQUENCE_COBS};
  return ss_uart_serial_send_and_finish_data(module, data, ENUM_ENTER_LEN);
}
int ss_send_enum_exit(uart_serial_module *module) {
  #define ENUM_EXIT_LEN 3
  static const uint8_t data[] = {0, 0xF1, ENUM_EXIT_LEN};
  return ss_uart_serial_send_and_finish_data(module, data, ENUM_EXIT_LEN);
}
int ss_send_enum_reset(uart_serial_module *module) {
  #define ENUM_RESET_LEN 3
  static const uint8_t data[] = {0, 0xF3, ENUM_RESET_LEN};
  return ss_uart_serial_send_and_finish_data(module, data, ENUM_RESET_LEN);
}
int ss_send_enum_select(uart_serial_module *module, uint8_t id[SMART_ID_LEN],
  uint8_t mask[SMART_ID_LEN]) {
  static const uint8_t len = 2*SMART_ID_LEN;  // Not COBS
  uint8_t data[2*SMART_ID_LEN];  // Not COBS
  for (int i = 0; i < SMART_ID_LEN; ++i) {
    data[i] = id[i];
    data[i+SMART_ID_LEN] = mask[i];
  }
  return ss_send_maintenance(module, 0xF2, data, len);
}
int ss_send_enum_unselect(uart_serial_module *module, uint8_t id[SMART_ID_LEN],
  uint8_t mask[SMART_ID_LEN]) {
  static const uint8_t len = 2*SMART_ID_LEN;  // Not COBS
  uint8_t data[2*SMART_ID_LEN];  // Not COBS
  for (int i = 0; i < SMART_ID_LEN; ++i) {
    data[i] = id[i];
    data[i+SMART_ID_LEN] = mask[i];
  }
  return ss_send_maintenance(module, 0xF4, data, len);
}
int ss_recieve_enum_any_unselected(uart_serial_module *module) {
  int r;
  switch ((r = uart_bus_logic_level(module))) {
    case 0: return 1;
    case 1: return 0;
    default: return r;
  }
  return r;
}
void ss_select_delay() {
  vTaskDelay(SENSOR_SELECT_DELAY / portTICK_RATE_MS);
}


/*
int ss_all_reset_bus() {
  static const
}
*/
transmit_allocations ss_send_active(uart_serial_module *module, uint8_t inband,
  uint8_t sample, uint8_t frame, uint8_t *data, uint8_t len) {
  transmit_allocations allocs = {
    .txn = NULL,
    .data = NULL,
  };

  // TODO(cduck): Support in-band signalling

  // TODO(cduck): Split up packet if too long.
  if (len > 12)return allocs;  // Too long for active packet
  if ((frame & 7) == 0)return allocs;  // Frame can never be zero

  uint8_t *data_cobs = (uint8_t*)pvPortMalloc(4+len);
  // Four extra for 0x00, sample/frame, len, and extra COBS byte.

  data_cobs[0] = 0x00;
  data_cobs[1] = ((!!inband) << 6) | ((sample & 7) << 3) | (frame & 7);
  data_cobs[2] = len+4;
  cobs_encode(data_cobs+3, data, len);

  allocs.txn = uart_serial_send_data(module, data_cobs, len+4);
  allocs.data = data_cobs;
  return allocs;
}
void ss_wait_until_done(uart_serial_module *module,
  transmit_allocations allocs) {
  while ((uart_serial_send_status(module, allocs.txn) !=
      UART_SERIAL_SEND_DONE) &&
      (uart_serial_send_status(module, allocs.txn) !=
        UART_SERIAL_SEND_ERROR)) {}
}
void ss_send_finish(uart_serial_module *module, transmit_allocations allocs) {
  if (allocs.data) vPortFree(allocs.data);
  if (allocs.txn) uart_serial_send_finish(module, allocs.txn);
}

