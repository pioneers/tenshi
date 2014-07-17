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
#include "inc/smartsensor/crc.h"

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




// Functions for getting sensor descriptor data
// Returns mallocated byte array.  You need to free the result
uint8_t *ss_read_descriptor(SSState *sensor, uint32_t *readLen) {
  *readLen = 0;

  uart_serial_module *bus = ssBusses[sensor->busNum];
  const uint8_t requestLen = 3;
  uint8_t request[3] = {0, 0, 2};  // Request two length bytes
  size_t recLen = 0;
  uint8_t *recData;
  const uint8_t prefixLen = 3;

  uint8_t temp[4];
  uint32_t allLen = 0, partLen = 0;
  uint8_t *allData = NULL;

  // Clear missed packets
  while (uart_serial_receive_packet(bus, &recLen, 0)) {}

  int success = ss_send_maintenance_to_sensor(sensor, SS_PACKET_DESCRIPTOR,
                  request, requestLen);  // Request two length bytes
  if (!success) return NULL;
  recData = uart_serial_receive_packet_timeout(bus, &recLen,
              SENSOR_REPLY_TIMEOUT);
  if (!recData) return NULL;
  if (recLen < prefixLen+3 || recData[1] != SS_PACKET_DESCRIPTOR) {
    vPortFree(recData);
    return NULL;
  }
  // At least two bytes decoded, at most 3 if extra data was recieved.
  cobs_decode(temp, recData+prefixLen, 4);
  vPortFree(recData);

  allLen = temp[0] + temp[1]*0x100;  // Little endian
  allData = pvPortMalloc(allLen);

  while (partLen < allLen) {
    request[0] = partLen & 0xFF;
    request[1] = (partLen >> 8) & 0xFF;
    request[2] = allLen-partLen > 255-prefixLen-1 ? 255-prefixLen-1
                                                  : allLen-partLen;
    int success = ss_send_maintenance_to_sensor(sensor, SS_PACKET_DESCRIPTOR,
                    request, requestLen);  // Request descriptor bytes
    if (!success) {
      vPortFree(allData);
      return NULL;
    }
    recData = uart_serial_receive_packet_timeout(bus, &recLen,
                SENSOR_REPLY_TIMEOUT);
    if (!recData || recLen < prefixLen+2  // Minimum 1 byte of descriptor data
        || recData[1] != SS_PACKET_DESCRIPTOR) {
      vPortFree(allData);
      vPortFree(recData);
      return NULL;
    }
    if (partLen + (recLen-prefixLen-1) > allLen)  // If too much data recieved
      recLen = allLen-partLen + prefixLen+1;
    cobs_decode(allData + partLen, recData+prefixLen, recLen-prefixLen);
    vPortFree(recData);

    partLen += recLen-prefixLen-1;
  }

  // TODO(cduck): Check the CRC
  uint8_t crc = crc8(0, allData, allLen);

  *readLen = allLen;
  return allData;
}
// Intreprets descriptor data and updates the SSState with it
// Returns 0 on fail
int ss_interpret_descriptor(SSState *sensor, uint8_t *data, uint32_t len) {
  if (sensor->hasReadDescriptor) return 0;
  uint8_t *end = data+len;
  data += 2;  // Ignore length bytes

  // Human-readable description
  if (data + 1 > end) return 0;
  sensor->descriptionLen = data[0];
  ++data;
  if (data + sensor->descriptionLen > end) return 0;
  vPortFree(sensor->description);
  sensor->description = pvPortMalloc(sensor->descriptionLen);
  if (!sensor->description) return 0;
  memcpy(sensor->description, data, sensor->descriptionLen);
  data += sensor->descriptionLen;

  // Chunks requested per sample numerator/denominator
  if (data + 2 > end) return 0;
  sensor->chunksNumerator = data[0];
  sensor->chunksDenominator = data[1];
  data += 2;

  // Channels
  if (data + 1 > end) return 0;
  uint8_t channelsNum = data[0];
  ++data;
  vPortFree(sensor->channels);
  sensor->channels = pvPortMalloc(sensor->channelsNum * sizeof(SSChannel*));
  if (!sensor->channels) return 0;

  for (sensor->channelsNum = 0; sensor->channelsNum < channelsNum;
      ++sensor->channelsNum) {
    if (data + 3 > end) return 0;
    if (data[1] + 3 > data[0]) return 0;
    if (data + data[0] > end) return 0;
    SSChannel *channel = pvPortMalloc(sizeof(SSChannel));
    if (!channel) return 0;

    channel->descriptionLen = data[1];
    uint8_t descriptorLen = data[0];
    data += 2;
    channel->description = pvPortMalloc(channel->descriptionLen);
    if (!channel->description) {
      vPortFree(channel);
      return 0;
    }
    memcpy(channel->description, data, channel->descriptionLen);
    data += channel->descriptionLen;

    channel->type = data[0];
    ++data;

    channel->additionalLen = descriptorLen - 3 - channel->descriptionLen;
    channel->additional = pvPortMalloc(channel->additionalLen);
    if (!channel->additional) {
      vPortFree(channel->description);
      vPortFree(channel);
      return 0;
    }
    memcpy(channel->additional, data, channel->additionalLen);
    data += channel->additionalLen;

    sensor->channels[sensor->channelsNum] = channel;

    if (sensor->channelsNum == 0) {
      sensor->primaryType = channel->type;
      sensor->hasReadDescriptor = 1;  // If it fails after this it is still
                                      // good enough.
    }
  }

  // Assumeing CRC has already been checked in ss_read_descriptor()

  sensor->hasReadDescriptor = 1;
  return 1;
}
// Does both of the above functions
// Returns 0 on fail
int ss_update_descriptor(SSState *sensor) {
  uint32_t len = 0;
  uint8_t *data = ss_read_descriptor(sensor, &len);
  int ret = 0;
  if (data && len > 0) {
    ret = ss_interpret_descriptor(sensor, data, len);
  }
  vPortFree(data);
  return ret;
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

  s->hasReadDescriptor = 0;
  s->descriptionLen = 0;
  s->description = NULL;
  s->chunksNumerator = 0;
  s->chunksDenominator = 0;
  s->channelsNum = 0;
  s->channels = NULL;
  s->primaryType = 0;

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

