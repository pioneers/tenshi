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

// This file handles maintenance packets

#include <string.h>
#include "inc/smartsensor/maintenance.h"

#define TYPE_PING_PONG    0xFE
#define TYPE_SLAVE_CONFIG 0xD0
#define TYPE_DESCRIPTOR 0xD1

uint8_t compare_ID(uint8_t *data);



void maintenancePacket(uint8_t type, uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen) {
  switch (type) {
    case TYPE_PING_PONG: maintenancePingPong(data, len, outData, outLen); break;
    case TYPE_SLAVE_CONFIG: maintenanceSlaveConfig(data, len, outData, outLen);
      break;
    case TYPE_DESCRIPTOR: maintenanceDescriptor(data, len, outData, outLen);
      break;
    default: break;
    // TODO(cduck): Add more maintenance packets
  }
}
void maintenancePingPong(uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen) {
  if (len < SMART_ID_LEN) return;  // Bad packet
  if (compare_ID(data)) {
    *outLen = len-SMART_ID_LEN;
    uint8_t *pongData = data+SMART_ID_LEN;
    for (int i = len-SMART_ID_LEN-1; i >= 0; --i) {
      outData[i] = pongData[i];
    }
    return;
  }
}
void maintenanceSlaveConfig(uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen) {
  if (len < SMART_ID_LEN) return;  // Bad packet
  if (compare_ID(data)) {
    memset(my_frames, 0, sizeof(my_frames));
    for (uint8_t i = 8; i < len; i += 2) {
      uint8_t sample = data[i], frame = data[i+1];
      if (sample == 0xFF && frame == 0xFF) {  // Allow everything
        memset(my_frames, 0xFF, sizeof(my_frames));
        return;
      } else if (sample == 0xFF) {  // Allow this frame in all samples.
        if (frame-SS_FIRST_FRAME < SS_NUM_FRAMES) {
          my_frames[frame-SS_FIRST_FRAME] = 0xFF;
        }
      } else if (frame == 0xFF) {  // Allow all frames in this sample
        for (uint8_t j = 0; j < SS_NUM_FRAMES; ++j) {
          my_frames[j] |= (uint8_t)(1 << sample);
        }
      } else {
        // Allow this frame in this sample.
        if (frame-SS_FIRST_FRAME < SS_NUM_FRAMES) {
          my_frames[frame-SS_FIRST_FRAME] |= (uint8_t)(1 << sample);
        }
      }
    }
    return;
  }
}

// Function for responding to a 0xD1 command
void maintenanceDescriptor(uint8_t *data, uint8_t len,
  uint8_t *outData, uint8_t *outLen) {
  // TODO(cduck): Make descriptor contain the descriptor
  uint8_t *descriptor = data;

  // Bad packet b/c len + start_address + len;
  if (len < SMART_ID_LEN + 3) return;
  if (compare_ID(data)) {
    uint16_t start = data[SMART_ID_LEN] |
                     (((uint16_t)data[SMART_ID_LEN + 1]) << 8);
    uint8_t returnLen = data[SMART_ID_LEN + 2];
    if (returnLen > 0xFF-4) returnLen = 0xFF-4;  // Too much for a packet
    uint16_t descriptorLen = descriptor[0] | (((uint16_t)descriptor[1]) << 8);
    if (start > descriptorLen) return;  // Past the end of the descriptor
    if (start + returnLen > descriptorLen) returnLen = descriptorLen - start;
    uint8_t *returnData = descriptor + start;
    *outLen = returnLen;
    memcpy(outData, returnData, returnLen);
  }
  return;
}


// Check if its my ID.
uint8_t compare_ID(uint8_t *data) {
  uint8_t val = 1;
  for (uint8_t i = 0; i < SMART_ID_LEN; i++) {
    val &= (smartID[i] == data[i]);
  }
  return val;
}
