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

#include "inc/smartsensor/enumeration.h"

#include "inc/uart_serial_driver.h"

#define MAX_BIT_DEPTH (SMART_ID_LEN*8)

typedef struct {
  KnownIDs *sensorIDs;
  uart_serial_module *bus;
  uint8_t busNum;
  uint8_t currID[SMART_ID_LEN];
  uint8_t bitDepth;
  uint8_t mask[SMART_ID_LEN];
} EnumInfo;

uint8_t idZero[] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t maskNone[] = {0, 0, 0, 0, 0, 0, 0, 0};

// Private functions
static void addIDToArray(EnumInfo *info);
static void findSensorID(EnumInfo *info);
static void getToState(EnumInfo *info);



void addIDToArray(EnumInfo *info) {
  if (info->sensorIDs->len < info->sensorIDs->maxLen) {
    info->sensorIDs->arr[info->sensorIDs->len] =
      ss_init_sensor(info->currID, info->busNum);
    info->sensorIDs->len++;
  } else {
    // TODO(cduck): Error or stop algorithm
  }
}

int enumerateSensors(KnownIDs *sensorIDs, uart_serial_module *bus,
  uint8_t busNum) {
  EnumInfo info = {
    .sensorIDs = sensorIDs,
    .bus = bus,
    .busNum = busNum,
    .currID = {0},
    .bitDepth = 0,
    .mask = {0},
  };

  // Confirm if anything is on the bus
  uint8_t unsel1 = 0;
  uint8_t unsel2 = 0;
  ss_send_enum_enter(bus);  // Enter enumeration
  ss_select_delay();
  unsel1 = ss_recieve_enum_any_unselected(bus);
  ss_send_enum_select(bus, idZero, maskNone);  // Select all sensors
  ss_select_delay();
  unsel2 = ss_recieve_enum_any_unselected(bus);

  int ret;
  if (unsel1 && !unsel2) {
    findSensorID(&info);  // Enumerate sensors
    ret = 1;
  } else {
    ret = 0;
  }

  ss_send_enum_exit(bus);  // Exit enumeration
  return ret;
}

void findSensorID(EnumInfo *info) {
  getToState(info);

  // If no sensors currently under search
  if (!ss_recieve_enum_any_unselected(info->bus)) {
    return;
  }

  // If found a sensor id
  if (info->bitDepth >= MAX_BIT_DEPTH) {
    addIDToArray(info);
    return;
  }

  info->mask[info->bitDepth/8] |= (1 << (7 - info->bitDepth % 8));
  info->currID[info->bitDepth/8] &= ~(1 << (7 - info->bitDepth % 8));
  info->bitDepth++;
  findSensorID(info);  // Recurse with bit=1
  info->currID[(info->bitDepth-1)/8] |= (1 << (7-(info->bitDepth-1) % 8));
  findSensorID(info);  // Recurse with bit=0
  info->bitDepth--;
  info->mask[info->bitDepth/8] &= ~(1 << (7 - info->bitDepth % 8));
}


// Unselects the currID up to the bit depth (by using the mask)
void getToState(EnumInfo *info) {
  ss_send_enum_select(info->bus, idZero, maskNone);  // Select all sensors
  ss_select_delay();
  // Unselect sensors currently under search
  ss_send_enum_unselect(info->bus, info->currID, info->mask);
  ss_select_delay();
}
