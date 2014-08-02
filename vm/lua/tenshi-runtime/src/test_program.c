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
static int get_device(lua_State *L) {
  size_t id_len;
  const char *id = lua_tolstring(L, -1, &id_len);
  const char *id_copy = malloc(id_len);
  memcpy(id_copy, id, id_len);
  lua_pop(L, 1);
  lua_pushlightuserdata(L, id_copy);
  return 1;
}

static int del_device(lua_State *L) {
  free(lua_touserdata(L, -1));
  lua_pop(L, 1);
  return 0;
}

static int query_dev_info(lua_State *L) {
  char *dev = lua_touserdata(L, 1);
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

static int global_actuator_data = 0;
static int set_testactuator_val(lua_State *L) {
  // Just set this global for now
  global_actuator_data = lua_tointeger(L, 2);
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

  // const char studentcode[] =
  //   "x = 42";

  // const char studentcode[] =
  //   "i = 0\n"
  //   "while true do\n"
  //   "    print(\"Hello world!\")\n"
  //   "    print(math.pi)\n"
  //   "    i = i + 1\n"
  //   "    if i > 300 then\n"
  //   "        get_own_actor():stop()\n"
  //   "    end\n"
  //   "end";

  // const char studentcode[] =
  //   "function b()\n"
  //   "    for i = 1,500,1 do\n"
  //   "        send(a_actor, i, a_actor, i, a_actor, i)\n"
  //   "        print(\"sent: \" .. i)\n"
  //   "    end\n"
  //   "end\n"
  //   "\n"
  //   "a_actor = get_own_actor()\n"
  //   "start_actor(b)\n"
  //   "for i = 1,500,1 do\n"
  //   "    print(\"recv: \" .. recv(a_actor))\n"
  //   "end";

  // const char studentcode[] =
  //   "install_trap_global()\n"   // Normally this would be further down
  //   "input_dev = get_device('input')\n"
  //   "input = triggers.changed(input_dev)\n"
  //   "output = get_device('output')\n"
  //   "full_mbox = __mboxinternal.create_independent_mbox()\n"
  //   "for i = 1,16 do\n"
  //   "    full_mbox:send({i})\n"
  //   "end\n"
  //   "\n"
  //   "print('units.mega = ' .. units.mega)\n"
  //   "print('units.kilo = ' .. units.kilo)\n"
  //   "print('units.mili = ' .. units.mili)\n"
  //   "print('units.micro = ' .. units.micro)\n"
  //   "print('units.nano = ' .. units.nano)\n"
  //   "print('units.inch = ' .. units.inch)\n"
  //   "print('units.pound = ' .. units.pound)\n"
  //   "print('units.deg = ' .. units.deg)\n"
  //   "\n"
  //   "while true do\n"
  //   // "    print('bullshit send')\n"
  //   // "    full_mbox:send({42}, {timeout = 5})\n"
  //   // "    print('done bullshit send')\n"
  //   // "    print('bullshit recv')\n"
  //   // "    x = recv({timeout = 10})\n"
  //   // "    print('bullshit done: ' .. tostring(x))\n"
  //   "    print('about to recv')\n"
  //   "    local x = input:recv()\n"
  //   "    if x == nil then x = 0 end\n"
  //   "    print('recv: ' .. x)\n"
  //   "    x = x + 1\n"
  //   "    print('sending using value')\n"
  //   "    output.value = x\n"
  //   "    print('sent: ' .. x)\n"
  //   "end";

  const char studentcode[] =
    "print(__runtimeinternal.get_device)";

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
    printf("-----> Sent into sensor: %d\n", i / 2);
    TenshiMainStackPushInt(s, i / 2);
    ret = MBoxSendSensor(s, "input", 5);
    printf("MBoxSendSensor: %d\n", ret);
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
