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
#include "inc/pindef.h"
#include "inc/id.h"
#include "inc/config.h"


#define SMART_BAUD 500000  // Smartsensor baud rate
#define SMART_ID_LEN 8   // Length of smartsensor personal ID

// May change
#define ACTIVE_PACKET_MAX_LEN 10  // Not including prefix or extra COBS byte

#define SS_NUM_SAMPLES 8  // 3 bits of resolution
#define SS_NUM_FRAMES 6
#define SS_FIRST_FRAME 1  // 1 indexed frame numbers



// ****Sensor Personal Data*** // to be a struct later.
extern uint8_t smartID[SMART_ID_LEN];
uint8_t my_frames[SS_NUM_FRAMES];
extern uint32_t sample_rate;  // hardcoded for now;



void ssInit();
void ssMainUpdate();



#endif  // INC_SMARTSENSOR_COMMON_H_
