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

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "inc/smartsensor/common.h"

// Public functions called from main.c
void initServo();
void activeServoRec(uint8_t *data, uint8_t len, uint8_t inband);
void activeServoSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband);

#endif  // INC_SERVO_H_
