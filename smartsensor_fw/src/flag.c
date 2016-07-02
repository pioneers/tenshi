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

// This file has team flag related functions

#include "inc/flag.h"

// Private global variables

// Private helper functions

// Public functions called from main.c
void initFlag() {
  DIGITAL_SET_LOW(BLUE_LED);
  DIGITAL_SET_LOW(YELLOW_LED);
}
void activeFlagRec(uint8_t *data, uint8_t len, uint8_t inband) {
  if (len < 1) return;

  if ((data[0] & 0xFC) == 0) {  // If only first two bits are used
    // Interpret first two bits as blue and yellow LED settings
    DIGITAL_SET(BLUE_LED, !!(data[0] & 1));
    DIGITAL_SET(YELLOW_LED, !!(data[0] & 2));
  } else {  // Interpret as mode byte
    switch (data[0]) {
      case FLAG_MODE_1:  // Not Implemented
        // break;
      default:  // Unknown mode
        DIGITAL_TOGGLE(BLUE_LED);
        DIGITAL_TOGGLE(YELLOW_LED);
        break;
    }
  }
}
void activeFlagSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  // Nothing to send
}
