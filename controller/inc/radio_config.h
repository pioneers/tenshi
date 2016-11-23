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

#ifndef INC_RADIO_CONFIG_H_
#define INC_RADIO_CONFIG_H_

#include "inc/FreeRTOS.h"
#include "inc/radio.h"
#include "inc/smartsensor/ssutil.h"
#include "radio_protocol_ng.h"   // NOLINT(build/include)

BaseType_t radioConfigInit();
void receiveConfigPort(config_port *port, size_t len);
#endif  // INC_RADIO_CONFIG_H_
