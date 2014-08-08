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

#ifndef INC_SMARTSENSOR_SMARTSENSOR_H_
#define INC_SMARTSENSOR_SMARTSENSOR_H_

#include "inc/FreeRTOS.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/uart_serial_driver.h"

#define SS_NUM_SAMPLES 8  // 3 bits of resolution
#define SS_NUM_FRAMES 6
#define SS_FIRST_FRAME 1  // 1 indexed frame numbers

// Set to whatever
#define SS_MAX_SENSORS_PER_BUS 32


void smartsensor_init();


portTASK_FUNCTION_PROTO(smartSensorTX, pvParameters);
portTASK_FUNCTION_PROTO(smartSensorRX, pvParameters);

#endif  // INC_SMARTSENSOR_SMARTSENSOR_H_
