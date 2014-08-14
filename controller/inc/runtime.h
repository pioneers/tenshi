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

#ifndef INC_RUNTIME_H_
#define INC_RUNTIME_H_

#include "inc/FreeRTOS.h"

#define RUNTIME_OK LUA_OK

typedef enum {
  RuntimeMessageNone = 0,
  RuntimeMessageNewCode,
  RuntimeMessageUbjson,
  RuntimeMessageGameMode,
} RuntimeMessageType;

typedef enum {
  RuntimeModeUninitialized = 0,
  RuntimeModeDisabled      = 1,
  RuntimeModeAutonomous    = 2,
  RuntimeModePaused        = 3,
  RuntimeModeTeleop        = 4,
} RuntimeMode;



extern volatile int gameMode;



BaseType_t runtimeInit();
void runtimeSendRadioMsg(RuntimeMessageType type, void* info, size_t infoLen);
// Takes responsibility for freeing
void runtimeRecieveUbjson(char *ubjson, size_t len);
void runtimeRecieveCode(char *code, size_t len);

char *readLastUbjson(size_t *len);  // Not thread safe


#endif  // INC_RUNTIME_H_
