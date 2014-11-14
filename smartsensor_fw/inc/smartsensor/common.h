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

#ifndef INC_SMARTSENSOR_COMMON_H_
#define INC_SMARTSENSOR_COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include "inc/pindef.h"


#define SMART_BAUD 500000  // Smartsensor baud rate
#define SMART_ID_LEN 8   // Length of smartsensor personal ID

// Sensor types
#define SENSOR_TYPE_DIGITAL 0x00
#define SENSOR_TYPE_ANALOG_IN 0x01
#define SENSOR_TYPE_SERVO    0x03
#define SENSOR_TYPE_SOLENOID 0x04
#define SENSOR_TYPE_GRIZZLY3 0x80
#define SENSOR_TYPE_BUZZER 0x81
#define SENSOR_TYPE_FLAG 0x82

// The sensor type is stored in flash memory directly after the ID.
#define SENSOR_TYPE pgm_read_byte(smartID+SMART_ID_LEN)

// Sensor modes
#define MODE_DISABLED 0x00
#define MODE_PAUSED   0x01
#define MODE_ACTIVE   0x02
#define MAX_MODE MODE_ACTIVE

// May change
#define ACTIVE_PACKET_MAX_LEN 10  // Not including prefix or extra COBS byte
#define DESCRIPTOR_MAX_LEN 1024

#define SS_NUM_SAMPLES 8  // 3 bits of resolution
#define SS_NUM_FRAMES 6
#define SS_FIRST_FRAME 1  // 1 indexed frame numbers



extern const uint8_t smartID[SMART_ID_LEN+1] PROGMEM;
extern const uint8_t descriptor[DESCRIPTOR_MAX_LEN] PROGMEM;

extern volatile uint8_t my_frames[SS_NUM_FRAMES];
extern volatile uint8_t gameMode;




void ssInit();
void ssMainUpdate();



#endif  // INC_SMARTSENSOR_COMMON_H_
