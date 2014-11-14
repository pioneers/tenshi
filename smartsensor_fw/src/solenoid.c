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

// This file has digital io smart sensor related functions

#include "inc/solenoid.h"

// Private global variables
uint8_t outValueSolenoid = 0;

// Private helper functions
void SolenoidSetValue(uint8_t val);


// Public functions called from main.c
void initSolenoid() {
  DIGITAL_SET_LOW(IN0);
  DIGITAL_SET_LOW(IN1);
  DIGITAL_SET_LOW(IN2);
  DIGITAL_SET_LOW(IN3);

  // TODO(tobinsarah): allow configuration as input/output for each relevant pin
  DIGITAL_SET_OUT(IN0);
  DIGITAL_SET_OUT(IN1);
  DIGITAL_SET_OUT(IN2);
  DIGITAL_SET_OUT(IN3);
}
void activeSolenoidRec(uint8_t *data, uint8_t len, uint8_t inband) {
  if (len >= 1) {
    outValueSolenoid = data[0];
  }
  switch (gameMode) {
    case MODE_ACTIVE: SolenoidSetValue(outValueSolenoid);
      break;
    case MODE_DISABLED: SolenoidSetValue(0);
      break;
    case MODE_PAUSED: SolenoidSetValue(0);  // Don't update outputs
    default: break;
  }
}
void activeSolenoidSend(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
*outLen = 0;
}





// Private helper functions
// TODO(karthik-shanmugam): update pins
void SolenoidSetValue(uint8_t val) {
  DIGITAL_SET(IN0, val & 1);  // UPDATE IN0
  DIGITAL_SET(IN1, val & 2);  // UPDATE IN1
  DIGITAL_SET(IN2, val & 4);  // UPDATE IN2
  DIGITAL_SET(IN3, val & 8);  // UPDATE IN3
}
