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

#include <stdio.h>
#include <string.h>

#include "inc/mboxlib.h"
#include "inc/runtime_entry.h"

// Dummy get_device, just takes the id and stores it
const char *hax_dev_sensor = "stestsensor";
const char *hax_dev_actuator = "atestactuator";
static int get_device(lua_State *L) {
  size_t id_len;
  const char *id = lua_tolstring(L, -1, &id_len);
  lua_pop(L, 1);
  printf("get_device: %s\n", id);
  // Awful hax
  if (strcmp(id, hax_dev_sensor) == 0) {
    lua_pushlightuserdata(L, hax_dev_sensor);
  } else if (strcmp(id, hax_dev_actuator) == 0) {
    lua_pushlightuserdata(L, hax_dev_actuator);
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int del_device(lua_State *L) {
  const char *dev = lua_touserdata(L, 1);
  lua_pop(L, 1);

  printf("del_device: %s\n", dev);

  return 0;
}

static int query_dev_info(lua_State *L) {
  const char *dev = lua_touserdata(L, 1);
  const char *query_type = lua_tostring(L, 2);
  lua_pop(L, 2);

  if (strcmp(query_type, "type") == 0) {
    // Are we a sensor or an actuator? In this test program, if id[0] is 's' we
    // are a sensor. Otherwise it's an actuator.
    if (dev[0] == 's') {
      lua_pushstring(L, "sensor");
    } else {
      lua_pushstring(L, "actuator");
    }
  } else if (strcmp(query_type, "dev") == 0) {
    // What is our device type? In this test program, it is id[1] to the first
    // embedded null or the end.
    lua_pushstring(L, dev + 1);
  } else {
    // Not supported
    lua_pushnil(L);
  }

  return 1;
}

static int global_sensor_data = 0;
static int get_testsensor_val(lua_State *L) {
  // Return this global for now
  lua_pop(L, 1);
  lua_pushinteger(L, global_sensor_data);

  return 1;
}

static int set_testactuator_val(lua_State *L) {
  const char *dev = lua_touserdata(L, 1);
  int val = lua_tointeger(L, 2);
  printf("<----- Got data out: %s = %d\n", dev, val);
  lua_pop(L, 2);

  return 0;
}

static const luaL_Reg testprogram_runtimeentries[] = {
  {"get_device", get_device},
  {"del_device", del_device},
  {"query_dev_info", query_dev_info},
  {"get_testsensor_val", get_testsensor_val},
  {"set_testactuator_val", set_testactuator_val},
  {NULL, NULL}
};

int main(int argc, char **argv) {
  printf("Hello world!\n");

  TenshiRuntimeState s = TenshiRuntimeInit();

  printf("Allocated state: %p\n", s);

  TenshiRegisterCFunctions(s, testprogram_runtimeentries);

  const char studentcode[] =
    "sensor = get_device('stestsensor')\n"
    "actuator = get_device('atestactuator')\n"
    "\n"
    "while true do\n"
    "    print('sensor is ' .. tostring(sensor.value))\n"
    "    actuator.value = sensor.value\n"
    "end";

  TenshiActorState a;

  int ret = MBoxCreateActuator(s, "output", 6);
  printf("MBoxCreateActuator: %d\n", ret);

  ret = MBoxCreateSensor(s, "input", 5);
  printf("MBoxCreateSensor: %d\n", ret);

  ret = LoadStudentcode(s, studentcode, strlen(studentcode), &a);
  printf("LoadStudentcode: %d, TenshiActorState: %p\n", ret, a);

  ret = ActorSetRunnable(a, 1);
  printf("ActorSetRunnable: %d\n", ret);

  int i = 0;

  while (i < 100) {
    printf("-----> Sent into sensor: %d\n", i);
    TenshiMainStackPushInt(s, i);
    global_sensor_data = i;
    ret = TenshiFlagSensor(s, hax_dev_sensor);
    printf("TenshiFlagSensor: %d\n", ret);

    ret = TenshiRunQuanta(s);
    printf("Ran quanta %d, ret = %d\n", i, ret);

    update_info *ui_orig = MBoxGetActuatorsChanged(s);
    printf("MBoxGetActuatorsChanged: %p\n", ui_orig);
    update_info *ui = ui_orig;
    while (ui) {
      printf("Actuator set: %s (%d values)\n", ui->id, ui->num_data);
      for (int j = 0; j < ui->num_data; j++) {
        ret = MBoxRecvActuator(s, ui->id, ui->id_len);
        printf("MBoxRecvActuator: %d\n", ret);
        int x = TenshiMainStackGetInt(s);
        printf("<----- Got data out: %d\n", x);
      }
      ui = ui->next;
    }
    MBoxFreeUpdateInfo(ui_orig);

    i++;
  }

  TenshiRuntimeDeinit(s);

  printf("Done!\n");

  return 0;
}
