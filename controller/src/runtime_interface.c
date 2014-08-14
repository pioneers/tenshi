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

// Interpreter
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <ngl_vm.h>
#include <ngl_buffer.h>
#include <ngl_package.h>

#include "inc/runtime_interface.h"
#include "inc/runtime.h"
#include "inc/smartsensor/smartsensor.h"
#include "inc/smartsensor/ssutil.h"
#include "inc/smartsensor/sstype.h"
#include "inc/led_driver.h"
#include "inc/button_driver.h"

#define LUA_REGISTER(FUNC) lua_register(L, #FUNC, lua_ ## FUNC)
#define LUA_REG(FUNC) {.name = #FUNC, .func = lua_ ## FUNC}



int isDeviceValid(SSChannel *dev);




void runtime_register(TenshiRuntimeState s) {
  luaL_Reg I[] = {
    LUA_REG(get_device),
    LUA_REG(del_device),
    LUA_REG(query_dev_info),

    LUA_REG(set_status_led_val),
    LUA_REG(get_button_val),
    LUA_REG(get_switch_val),
    LUA_REG(set_led_val),
    LUA_REG(get_analog_val),
    LUA_REG(set_analog_val),
    LUA_REG(set_grizzly_val),
    {NULL, NULL}
  };

  TenshiRegisterCFunctions(s, I);
}
void lua_register_all(lua_State *L) {
  LUA_REGISTER(get_device);
  LUA_REGISTER(del_device);
  LUA_REGISTER(query_dev_info);

  LUA_REGISTER(set_status_led_val);
  LUA_REGISTER(get_button_val);
  LUA_REGISTER(get_switch_val);
  LUA_REGISTER(set_led_val);
  LUA_REGISTER(get_analog_val);
  LUA_REGISTER(set_analog_val);
  LUA_REGISTER(set_grizzly_val);
}

//  Runtime required

int lua_get_device(lua_State *L) {
  const char *str = lua_tolstring(L, -1, NULL);
  lua_pop(L, 1);

  uint8_t id[SMART_ID_LEN];
  int idLen = 0;
  int chan = -1;
  int ret = sscanf(str, SMART_ID_SCANF "%n-%x",
              id, id+1, id+2, id+3, id+4, id+5, id+6, id+7, &idLen, &chan);
  int valid = ret >= SMART_ID_LEN && idLen == SMART_ID_LEN*2;

  SSState *sensor = NULL;
  SSChannel *channel = NULL;
  if (valid) {
    sensor = ss_find_sensor(id);
  }

  if (sensor) {
    if (chan < 0) chan = 0;  // Default to first channel
    // Skip all protected (not accessable by students) channels
    for (int i = 0, n = 0; i < sensor->channelsNum && channel == NULL; i++) {
      if (sensor->channels[i]->isProtected) continue;
      if (n == chan) channel = sensor->channels[n];
      n++;
    }
  }

  if (channel) {
    lua_pushlightuserdata(L, channel);
  } else {
    lua_pushnil(L);
  }

  return 1;
}
int lua_del_device(lua_State *L) {
  lua_pop(L, 1);
  // Do nothing to delete
  return 0;
}
int lua_query_dev_info(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  const char *query_type = lua_tostring(L, 2);
  lua_pop(L, 2);

  if (strcmp(query_type, "type") == 0) {
    // TODO(cduck): Handle device that is both an actuator and a sensor
    if (dev->isActuator) {
      lua_pushstring(L, "actuator");
    } else if (dev->isSensor) {
      lua_pushstring(L, "sensor");
    } else {
      lua_pushnil(L);
    }
  } else if (strcmp(query_type, "dev") == 0) {
    // What is our device type name?
    const char *name = ss_channel_name(dev);
    if (name) {
      lua_pushstring(L, name);
    } else {
      lua_pushnil(L);
    }
  } else {
    // Not supported
    lua_pushnil(L);
  }

  return 1;
}



// Sensor specific

int isDeviceValid(SSChannel *dev) {
  return dev != NULL && !dev->isProtected;
}

int lua_set_status_led_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  int val = lua_tointeger(L, 2);
  lua_pop(L, 2);
  led_driver_set_mode(PATTERN_JUST_RED);
  led_driver_set_fixed(val, 0b111);

  // TODO(cduck): Return error code?
  return 0;
}

int lua_get_button_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  lua_pop(L, 1);
  int button = button_driver_get_button_state(0);

  lua_pushinteger(L, button);
  return 1;
}

int lua_get_switch_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  lua_pop(L, 1);
  if (isDeviceValid(dev)) {
    lua_pushnil(L);
    return 1;
  }

  lua_pushnumber(L, ss_get_switch_val(dev));
  return 1;
}
int lua_set_led_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  uint8_t val = (uint8_t)lua_tointeger(L, 2);
  lua_pop(L, 2);
  if (isDeviceValid(dev)) {
    return 0;
  }

  ss_set_led_val(dev, val);

  return 0;
}

int lua_get_analog_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  lua_pop(L, 1);
  if (isDeviceValid(dev)) {
    lua_pushnil(L);
    return 1;
  }

  lua_pushnumber(L, ss_get_analog_val(dev));
  return 1;
}
int lua_set_analog_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  double val = (uint8_t)lua_tonumber(L, 2);
  lua_pop(L, 2);
  if (isDeviceValid(dev)) {
    return 0;
  }

  ss_set_analog_val(dev, val);

  return 0;
}

int lua_set_grizzly_val(lua_State *L) {
  SSChannel *dev = lua_touserdata(L, 1);
  double val = lua_tonumber(L, 2);
  int hasMode = 0;
  uint8_t mode = (uint8_t)lua_tointegerx(L, 3, &hasMode);  // Optional argument
  lua_pop(L, 2);
  if (isDeviceValid(dev)) {
    return 0;
  }

  if (!hasMode) {
    mode = GRIZZLY_DEFAULT_MODE;
  }
  ss_set_grizzly_val(dev, mode, val);

  return 0;
}



// Not hooked up to lua
// Sets the game mode channel on all smart sensors
void setAllSmartSensorGameMode(uint8_t mode) {
  for (int i = 0; i < numSensors; i++) {
    SSState *sensor = sensorArr[i];
    SSChannel *channel = NULL;
    for (int c = 0; c < sensor->channelsNum && channel == NULL; c++) {
      if (sensor->channels[c]->type == CHANNEL_TYPE_MODE) {
        channel = sensor->channels[c];
      }
    }
    if (channel) {
      ss_set_mode_val(channel, mode);
    }
  }
}

