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

#ifndef INC_SMARTSENSOR_ENUMERATION_H_
#define INC_SMARTSENSOR_ENUMERATION_H_

#include "inc/smartsensor/common.h"

extern uint8_t enumerating;

void enumerationPacket(uint8_t type, uint8_t *data, uint8_t len);
void enumerationEnter(uint8_t *data, uint8_t len);
void enumerationExit(uint8_t *data, uint8_t len);
void enumerationReset(uint8_t *data, uint8_t len);
void enumerationSelect(uint8_t *data, uint8_t len);
void enumerationUnselect(uint8_t *data, uint8_t len);

#endif  // INC_SMARTSENSOR_ENUMERATION_H_
