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

#include "inc/smartsensor/ss_type.h"
#include "inc/smartsensor/common.h"

#include "inc/digital.h"
#include "inc/analog_in.h"
#include "inc/buzzer.h"
#include "inc/flag.h"

void ssInitType() {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      initDigital();
      break;
    case SENSOR_TYPE_ANALOG_IN:
      initAnalogIn();
      break;
    case SENSOR_TYPE_BUZZER:
      initBuzzer();
      break;
    case SENSOR_TYPE_FLAG:
      initFlag();
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveSend(uint8_t *decodedBuffer, uint8_t *pacLen, uint8_t *inband) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      activeDigitalSend(decodedBuffer, pacLen,  inband);
      break;
    case SENSOR_TYPE_ANALOG_IN:
      activeAnalogInSend(decodedBuffer, pacLen, inband);
      break;
    case SENSOR_TYPE_BUZZER:
      activeBuzzerSend(decodedBuffer, pacLen, inband);
      break;
    case SENSOR_TYPE_FLAG:
      activeFlagSend(decodedBuffer, pacLen, inband);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}

void ssActiveInRec(uint8_t *decodedBuffer, uint8_t dataLen, uint8_t inband) {
  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      activeDigitalRec(decodedBuffer, dataLen, inband);
      break;
    case SENSOR_TYPE_ANALOG_IN:
      activeAnalogInRec(decodedBuffer, dataLen, inband);
      break;
    case SENSOR_TYPE_BUZZER:
      activeBuzzerRec(decodedBuffer, dataLen, inband);
      break;
    case SENSOR_TYPE_FLAG:
      activeFlagRec(decodedBuffer, dataLen, inband);
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }
}
