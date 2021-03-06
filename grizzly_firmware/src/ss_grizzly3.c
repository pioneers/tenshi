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

// This file has Grizzly 3.0 smart sensor related functions

#include "inc/ss_grizzly3.h"

#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "inc/i2c_register.h"
#include "inc/twi_state_machine.h"
#include "inc/control_loop.h"

// Private global variables
uint8_t replyRegister = 0;
uint8_t replyLen = 0xFF;

uint8_t modeSet = 0;
uint8_t speedSet[4] = {0};


// Private helper functions
void grizzly3SetValue(uint8_t mode, uint8_t speed[4]);


void initGrizzly3() {
  // Nothing needed
}
void activeGrizzly3Rec(uint8_t *data, uint8_t len, uint8_t inband) {
  last_i2c_update = get_uptime_dangerous();

  uint8_t apply = 0;

  if (!inband) {  // Expecting mode byte and 4 optional speed bytes.
    if (len > 0) {
      modeSet = data[0];  // Set grizzly mode
    }
    if (len >= 5) {
      speedSet[0] = data[1];  // Set speed
      speedSet[1] = data[2];
      speedSet[2] = data[3];
      speedSet[3] = data[4];
    }
    apply = 1;
  } else {  // Same as USB protocol
    uint8_t reg = data[0];
    uint8_t regLen = data[1] & 0x7F;
    uint8_t rw = data[1] & 0x80;
    if (rw) {
      // Write request
      if (regLen > len - 2)
        regLen = len - 2;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t i = 0; i < regLen; i++) {
          if (reg + i == REG_PWM_MODE) {
            modeSet = data[i + 2];
          } else if (reg + i >= REG_TARGET_SPEED_NEW &&
                     reg + i < REG_TARGET_SPEED_NEW + 4) {
            speedSet[reg + i - REG_TARGET_SPEED_NEW] = data[i + 2];
          } else if (reg + i == REG_APPLY_NEW_SPEED) {
            apply = 1;
          }
          set_i2c_reg(reg + i, data[i + 2]);
        }
      }
    } else {
      // Read request
      replyRegister = reg;
      replyLen = regLen;
    }
  }

  switch (gameMode) {
    case MODE_ACTIVE:
      if (apply) grizzly3SetValue(modeSet, speedSet);
      break;
    case MODE_DISABLED: grizzly3SetValue(0, speedSet);
      break;
    case MODE_PAUSED:
      if ((pwm_mode & MODE_SPEED_MASK) == MODE_POS_PID ||
          !(pwm_mode & MODE_ENABLE_MASK)) {
        // Don't update value; continue holding position or stay disabled
      } else {
        // TODO(cduck): Is it best to brake or coast on pause?
        // Set speed to zero and mode to controlled brake
        uint8_t sp = {0, 0, 0, 0};
        grizzly3SetValue(0x23, sp);
      }
      break;
    default: break;
  }
}
void activeGrizzly3Send(uint8_t *outData, uint8_t *outLen, uint8_t *inband) {
  if (replyLen < 0xFF) {
    if (replyLen > ACTIVE_PACKET_MAX_LEN - 2)
      replyLen = ACTIVE_PACKET_MAX_LEN - 2;
    outData[0] = replyRegister;
    outData[1] = replyLen;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      for (uint8_t i = 0; i < replyLen; i++) {
        outData[i + 2] = provide_i2c_reg(replyRegister + i);
      }
    }

    *inband = 1;
    *outLen = replyLen+2;

    replyLen = 0xFF;
  } else {
    // Haven't yet decided what to reply with when not in-band
  }
}


void grizzly3SetValue(uint8_t mode, uint8_t speed[4]) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    set_i2c_reg(REG_PWM_MODE, mode);  // Set mode
    set_i2c_reg(REG_TARGET_SPEED_NEW,   speed[0]);  // Set speed
    set_i2c_reg(REG_TARGET_SPEED_NEW+1, speed[1]);
    set_i2c_reg(REG_TARGET_SPEED_NEW+2, speed[2]);
    set_i2c_reg(REG_TARGET_SPEED_NEW+3, speed[3]);
    set_i2c_reg(REG_APPLY_NEW_SPEED, 0);  // Update Grizzly
  }
}
