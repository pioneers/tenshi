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

#include "inc/smartsensor/sstype.h"

uint8_t ss_channel_is_actuator(SSChannel *channel) {
  switch (channel->type) {
    case CHANNEL_TYPE_MODE:
      return 1;
    case SENSOR_TYPE_DIGITAL:
      return !!(channel->additional[0] & 0x0C);  // Either can output bit
    case SENSOR_TYPE_ANALOG_IN:
      return 0;
    case SENSOR_TYPE_GRIZZLY3:
      return 1;
    case SENSOR_TYPE_BUZZER:
      return 0;
    case SENSOR_TYPE_FLAG:
      return 1;
    // TODO(cduck): Add more types
    default: return 0;
  }
}
uint8_t ss_channel_is_sensor(SSChannel *channel) {
  switch (channel->type) {
    case CHANNEL_TYPE_MODE:
      return 0;
    case SENSOR_TYPE_DIGITAL:
      return !!(channel->additional[0] & 0x01);  // Can input bit
    case SENSOR_TYPE_ANALOG_IN:
      return 1;
    case SENSOR_TYPE_GRIZZLY3:
      return 0;  // Will change when feedback is implemented
    case SENSOR_TYPE_BUZZER:
      return 1;
    case SENSOR_TYPE_FLAG:
      return 0;
    // TODO(cduck): Add more types
    default: return 0;
  }
}
uint8_t ss_channel_is_protected(SSChannel *channel) {
  switch (channel->type) {
    case CHANNEL_TYPE_MODE: return 1;
    case SENSOR_TYPE_BUZZER: return 1;
    case SENSOR_TYPE_FLAG: return 1;
    // TODO(cduck): Add more types
    default: return 0;
  }
}
const char * ss_channel_name(SSChannel *channel) {
  switch (channel->type) {
    case CHANNEL_TYPE_MODE: return "mode";
    case SENSOR_TYPE_DIGITAL:
      if (!!(channel->additional[0] & 0x0C)) {  // Either can output bit
        return "led";
      } else {
        return "switch";
      }
    case SENSOR_TYPE_ANALOG_IN: return "analog";
    case SENSOR_TYPE_GRIZZLY3: return "grizzly";
    case SENSOR_TYPE_BUZZER: return "buzzer";
    case SENSOR_TYPE_FLAG: return "flag";
    // TODO(cduck): Add more types
    default: return NULL;
  }
}
uint8_t ss_channel_out_length(SSChannel *channel) {
  switch (channel->type) {
    case CHANNEL_TYPE_MODE:
      return 1;
    case SENSOR_TYPE_DIGITAL:
      return 1;  // TODO(cduck): More logic
    case SENSOR_TYPE_ANALOG_IN:
      return 0;
    case SENSOR_TYPE_GRIZZLY3:
      return 5;
    case SENSOR_TYPE_BUZZER:
      return 0;
    case SENSOR_TYPE_FLAG:
      return 1;
    // TODO(cduck): Add more types
    default: return 0;
  }
}
uint8_t ss_channel_in_length(SSChannel *channel) {
  switch (channel->type) {
    case CHANNEL_TYPE_MODE:
      return 0;
    case SENSOR_TYPE_DIGITAL:
      return 1;  // TODO(cduck): More logic
    case SENSOR_TYPE_ANALOG_IN:
      return 2;
    case SENSOR_TYPE_GRIZZLY3:
      return 0;
    case SENSOR_TYPE_BUZZER:
      return 8;
    case SENSOR_TYPE_FLAG:
      return 0;
    // TODO(cduck): Add more types
    default: return 0;
  }
}



void ss_set_mode_val(SSChannel *channel, uint8_t val) {
  ss_set_value(channel, &val, 1);
}
float ss_get_generic_val(SSChannel *channel) {
    switch (channel->type){
        case SENSOR_TYPE_DIGITAL:
            return ss_get_switch_val(channel);
        case SENSOR_TYPE_ANALOG_IN:
            return ss_get_analog_val(channel);
        default:
            return 0;
    
    }
}
void ss_set_led_val(SSChannel *channel, uint8_t val) {
  ss_set_value(channel, &val, 1);
}
uint8_t ss_get_switch_val(SSChannel *channel) {
  // If no value has been recieved yet default to 0.
  uint8_t result = 0;
  ss_get_value(channel, &result, 1);
  return result;
}

void ss_set_analog_val(SSChannel *channel, double num) {
  if (num < 0.) num = 0.;
  if (num > 1.) num = 1.;

  uint8_t bitsPerSample = 8;
  if (channel->additionalLen >= 3) bitsPerSample = channel->additional[2];
  if (bitsPerSample < 1) bitsPerSample = 8;
  if (bitsPerSample > 64) bitsPerSample = 8;
  double factor = (1 << bitsPerSample) - 1;

  uint64_t val = (uint64_t)(num * factor);
  uint8_t val_len = (bitsPerSample + 7) / 8;
  uint8_t data[8] =
    {val, val>>8, val>>16, val>>24, val>>32, val>>40, val>>48, val>>56};

  ss_set_value(channel, data, val_len);
}
double ss_get_analog_val(SSChannel *channel) {
  uint8_t bitsPerSample = 8;
  if (channel->additionalLen >= 3) bitsPerSample = channel->additional[2];
  if (bitsPerSample < 1) bitsPerSample = 10;
  if (bitsPerSample > 64) bitsPerSample = 10;
  double factor = (1 << bitsPerSample) - 1;

  uint8_t val_len = (bitsPerSample + 7) / 8;
  // If no value has been recieved yet default to 0.
  uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  ss_get_value(channel, data, val_len);

  uint64_t result = 0;
  for (int i = 0; i < val_len; ++i) {
    result |= ((uint64_t)data[i]) << (i*8);
  }
  return result/factor;
}

void ss_set_grizzly_val(SSChannel *channel, uint8_t mode, double speed) {
  uint8_t data[5];
  data[0] = mode;
  int32_t speed32 = (int32_t)(speed * 65536.f);
  data[1] = speed32;
  data[2] = speed32 >> 8;
  data[3] = speed32 >> 16;
  data[4] = speed32 >> 24;
  ss_set_value(channel, data, 5);
}
