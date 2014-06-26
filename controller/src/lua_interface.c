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

#include "inc/lua_interface.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/led_driver.h"
#include "inc/button_driver.h"

#define LUA_REGISTER(FUNC) lua_register(L, #FUNC, lua_ ## FUNC)



void lua_register_all(lua_State *L) {
  LUA_REGISTER(set_led);
  LUA_REGISTER(get_button);
  LUA_REGISTER(get_digital);
  LUA_REGISTER(set_digital);
  LUA_REGISTER(get_analog);
  LUA_REGISTER(set_analog);
  LUA_REGISTER(set_motor);
}

int lua_set_led(lua_State *L) {
  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(lua_tonumber(L, 1), 0b111);

  int ret = 1;  // 1 means ok.
  lua_pushnumber(L, ret);
  return 1;
}

int lua_get_button(lua_State *L) {
  int button = button_driver_get_button_state(0);

  lua_pushnumber(L, button);
  return 1;
}

int lua_get_digital(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  lua_pushnumber(L, ss_get_digital_value(sensorIndex));
  return 1;
}
int lua_set_digital(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  uint8_t sensorVal = (uint8_t)lua_tonumber(L, 2);

  ss_set_digital_value(sensorIndex, sensorVal);

  lua_pushnumber(L, 1);  // 1 means ok.
  return 1;
}

int lua_get_analog(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  lua_pushnumber(L, ss_get_analog_value(sensorIndex));
  return 1;
}
int lua_set_analog(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  unsigned int sensorVal = (unsigned int)lua_tonumber(L, 2);

  ss_set_analog_value(sensorIndex, sensorVal);

  lua_pushnumber(L, 1);  // 1 means ok.
  return 1;
}

int lua_set_motor(lua_State *L) {
  int sensorIndex = lua_tonumber(L, 1);
  uint8_t mode = GRIZZLY_DEFAULT_MODE;
  double speed = lua_tonumber(L, 2);

  ss_set_motor_value(sensorIndex, mode, speed);

  lua_pushnumber(L, 1);  // 1 means ok.
  return 1;
}

