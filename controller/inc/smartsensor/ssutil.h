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

#ifndef INC_SMARTSENSOR_SSUTIL_H_
#define INC_SMARTSENSOR_SSUTIL_H_

#include <string.h>

#include "inc/FreeRTOS.h"
#include "inc/semphr.h"
#include "inc/event_groups.h"

#include "inc/uart_serial_driver.h"


#define GRIZZLY_DEFAULT_MODE 0x03  // No PID

#define SMART_ID_LEN 8   // Length of smartsensor personal ID
#define SMART_ID_SCANF "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx"
#define SMART_ID_SCANF_LEN 40  // This must equal the length of the string above
#if SMART_ID_SCANF_LEN != SMART_ID_LEN * 5
  #error Length of smart senosr ID scanf format string does  /* NOLINT(*) */\
  not match SMART_ID_LEN.  /* NOLINT(*) */
#endif

#define SS_MAX_ACTIVE_LEN (16-4)  // Most bytes that can be sent to a sensor
                                  // during active bus mode.

// Timeout
#define SENSOR_WAIT_TIME (1000/portTICK_PERIOD_MS)  // 1 seconds
// Time for sensor to respond to enumeration select command
#define SENSOR_SELECT_DELAY 1  // 1 ms
#define SENSOR_REPLY_TIMEOUT 7  // ms
#define SEMAPHORE_WAIT_TIME (1000/portTICK_PERIOD_MS)

// Smart sensor packet type definitions
#define SS_PACKET_DESCRIPTOR 0xD1

#define MAGIC_SEQUENCE_LEN 21
#define MAGIC_SEQUENCE 0xE6, 0xAD, 0xBB, 0xE3, 0x82, 0x93, 0xE3, 0x81, 0xA0, \
                       0xE4, 0xB8, 0x96, 0xE7, 0x95, 0x8C, 0xE6, 0x88, 0xA6, \
                       0xE7, 0xB7, 0x9A /* NOLINT(*) */
#define MAGIC_SEQUENCE_COBS \
                       0x16, 0xe6, 0xad, 0xbb, 0xe3, 0x82, 0x93, 0xe3, 0x81, \
                       0xa0, 0xe4, 0xb8, 0x96, 0xe7, 0x95, 0x8c, 0xe6, 0x88, \
                       0xa6, 0xe7, 0xb7, 0x9a /* NOLINT(*) */



typedef struct {
  uint8_t descriptionLen;
  char *description;
  uint8_t type;
  uint8_t additionalLen;
  uint8_t *additional;  // Sensor type specific data
                        // TODO(cduck): Actually use the additional information

  // Communication buffers and locks
  xSemaphoreHandle outLock;  // Same out lock for the entire sensor
  size_t outgoingLen;  // Calculated from descriptor; don't modify
  uint8_t *outgoingBytes;  // Points to the middle of the sensor's outgoingBytes
  xSemaphoreHandle inLock;  // Same in lock for the entire sensor
  size_t incomingLen;  // Calculated from descriptor; don't modify
  uint8_t *incomingBytes;  // Points to the middle of the sensor's incomingBytes

  uint8_t isActuator;
  uint8_t isSensor;
  uint8_t isProtected;  // True if student code is not allowed access
} SSChannel;

typedef struct {
  // Sensor identification
  uint8_t id[SMART_ID_LEN];
  uint8_t busNum;

  // Communication buffers and locks
  xSemaphoreHandle outLock;  // Same out lock for the entire sensor
  size_t outgoingLen;  // Calculated from descriptor; don't modify
  uint8_t *outgoingBytes;
  xSemaphoreHandle inLock;  // Same in lock for the entire sensor
  size_t incomingLen;  // Calculated from descriptor; don't modify
  uint8_t *incomingBytes;

  uint8_t hasReadDescriptor;
  // Descriptor data (not valid until hasReadDescriptor==1)
  uint8_t descriptionLen;
  char *description;  // Human readable
  // TODO(cduck): Figure out what to do with "chunks requested per sample
  //   numerator/denominator"
  uint8_t chunksNumerator;
  uint8_t chunksDenominator;
  uint8_t channelsNum;
  SSChannel **channels;  // TODO(cduck): Currently only know how to handle one
  uint8_t primaryType;  // Used until I figure out what to do about mutiple
                        //   channels
} SSState;


typedef struct _transmit_allocations {
  void *txn;
  void *data;
} transmit_allocations;

typedef struct {
  SSState **arr;
  size_t len;
  size_t maxLen;
} KnownIDs;


extern EventGroupHandle_t ssBusEventGroup;

extern size_t numSensors;
extern size_t numSensorsAlloc;
extern SSState **sensorArr;
extern xSemaphoreHandle sensorArrLock;


// Functions to enable external code to set and read sensors
void ss_set_value(SSChannel *channel, uint8_t *data, size_t len);
// len is the maximum number to be stored in the data buffer.
// Returns how many extra bytes were not stored in data buffer.
int ss_get_value(SSChannel *channel, uint8_t *data, size_t len);


// Helper functions for smartsensor.c
SSState *ss_init_sensor(uint8_t id[SMART_ID_LEN], uint8_t busNum);
// Returns the index of the sensor
size_t ss_add_new_sensor(uint8_t id[SMART_ID_LEN], uint8_t busNum);
size_t ss_add_sensor(SSState *sensor);
size_t ss_add_sensors(KnownIDs *sensors);
SSState *ss_find_sensor(uint8_t id[SMART_ID_LEN]);

void ss_recieved_data_for_sensor(SSState *s, uint8_t *data, size_t len,
  uint8_t inband);
// Assuming the sensor is already locked
int checkOutgoingBytes(SSChannel *sensor, uint8_t requiredLen);
int checkIncomingBytes(SSChannel *sensor, uint8_t requiredLen);


// Functions for getting sensor descriptor data
// Returns mallocated byte array.  You need to free the result
uint8_t *ss_read_descriptor(SSState *sensor, uint32_t *readLen);
// Intreprets descriptor data and updates the SSState with it
int ss_interpret_descriptor(SSState *sensor, uint8_t *data, uint32_t len);
// Does both of the above functions
int ss_update_descriptor(SSState *sensor);


// Helper functions for smartsensor.c
int ss_uart_serial_send_and_finish_data(uart_serial_module *module,
  const uint8_t *data, size_t len);
int ss_all_uart_serial_send_and_finish_data(const uint8_t *data, size_t len);
int ss_send_maintenance(uart_serial_module *module, uint8_t type,
  const uint8_t *data, uint8_t len);
int ss_send_maintenance_to_sensor(SSState *sensor, uint8_t type,
  const uint8_t *data, uint8_t len);
int ss_all_send_maintenance(uint8_t type, const uint8_t *data, uint8_t len);
int ss_send_ping_pong(SSState *sensor, const uint8_t *data, uint8_t len);

int ss_send_enum_enter(uart_serial_module *module);
int ss_send_enum_exit(uart_serial_module *module);
int ss_send_enum_reset(uart_serial_module *module);
int ss_send_enum_select(uart_serial_module *module, uint8_t id[SMART_ID_LEN],
  uint8_t mask[SMART_ID_LEN]);
int ss_send_enum_unselect(uart_serial_module *module, uint8_t id[SMART_ID_LEN],
  uint8_t mask[SMART_ID_LEN]);
// Returns 1 if any sensors are still driving a logic level 0 onto the bus.
int ss_recieve_enum_any_unselected(uart_serial_module *module);
void ss_select_delay();

transmit_allocations ss_send_active(uart_serial_module *module, uint8_t inband,
  uint8_t sample, uint8_t frame, uint8_t *data, uint8_t len);
void ss_wait_until_done(uart_serial_module *module,
  transmit_allocations allocs);
void ss_send_finish(uart_serial_module *module, transmit_allocations allocs);


#endif  // INC_SMARTSENSOR_SSUTIL_H_
