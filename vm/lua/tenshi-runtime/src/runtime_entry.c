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

#include "inc/runtime_entry.h"

#include <stdlib.h>
#include "inc/actor_sched.h"
#include "inc/actorlib.h"
#include "inc/mboxlib.h"
#include "inc/runtime_internal.h"
#include "inc/runtime_libs.h"
#include "lua.h"        // NOLINT(build/include)
#include "lualib.h"     // NOLINT(build/include)
#include "lauxlib.h"    // NOLINT(build/include)
#include "threading.h"  // NOLINT(build/include)
#include "../actuator_actor.lc.h"   // NOLINT(build/include)
#include "../get_device.lc.h"       // NOLINT(build/include)
#include "../sensor_actor.lc.h"     // NOLINT(build/include)
#include "../trap_global.lc.h"      // NOLINT(build/include)

static int tenshi_open_get_device(lua_State *L) {
  luaL_loadbuffer(L, get_device_lc, sizeof(get_device_lc), "get_device.lua");
  lua_pcall(L, 0, 0, 0);
  return 0;
}

static int tenshi_open_trap_global(lua_State *L) {
  luaL_loadbuffer(L, trap_global_lc, sizeof(trap_global_lc),
    "trap_global.lua");
  lua_pcall(L, 0, 0, 0);
  return 0;
}

#ifdef __arm__

// Check if the passed-in address is in flash memory or not
extern const char __flash_start;
extern const char __flash_end;
int lua_arm_checkxip(const void *ptr) {
  return (ptr >= &__flash_start) && (ptr < &__flash_end);
}

#endif

TenshiRuntimeState TenshiRuntimeInit(void) {
  TenshiRuntimeState ret;

  ret = (TenshiRuntimeState)(malloc(sizeof(struct _TenshiRuntimeState)));
  if (!ret) return NULL;

  ret->L = luaL_newstate();
  if (!ret->L) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Register checkxip function on ARM
  #ifdef __arm__
  lua_setcheckxip(ret->L, lua_arm_checkxip);
  #endif

  // Load libraries
  TenshiRuntime_openlibs_phase1(ret->L);

  lua_gc(ret->L, LUA_GCCOLLECT, 0);

  // Load actor library. This is special because it loads into the global
  // scope and not into a specific module.
  tenshi_open_actor(ret->L);
  // Load mailbox library. Similar thing.
  tenshi_open_mbox(ret->L);
  // Load get_device implementation.
  tenshi_open_get_device(ret->L);
  // Load install_trap_global implementation.
  tenshi_open_trap_global(ret->L);

  TenshiRuntime_openlibs_phase2(ret->L);

  // Set up actor scheduler
  lua_pushcfunction(ret->L, ActorSchedulerInit);
  if (lua_pcall(ret->L, 0, 0, 0) != LUA_OK) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Set up preemption trick
  threading_setup(ret->L);

  // Store ourselves into the registry
  lua_pushstring(ret->L, RIDX_RUNTIMESTATE);
  lua_pushlightuserdata(ret->L, ret);
  lua_settable(ret->L, LUA_REGISTRYINDEX);

  // Load the code for the actor that handles reading sensor updates and
  // updating sensor mailboxes
  ret->sensor_actor = ActorCreate(ret);
  if (!ret->sensor_actor) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }
  int ret_ = luaL_loadbuffer(ret->sensor_actor->L,
    sensor_actor_lc, sizeof(sensor_actor_lc), "sensor_actor.lua");
  if (ret_ != LUA_OK) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Load the code for the actor that handles reading actuator mailboxes and
  // updating actuators.
  ret->actuator_actor = ActorCreate(ret);
  if (!ret->actuator_actor) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }
  ret_ = luaL_loadbuffer(ret->actuator_actor->L,
    actuator_actor_lc, sizeof(actuator_actor_lc), "actuator_actor.lua");
  if (ret_ != LUA_OK) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Construct the RIDX_CHANGED_SENSORS table
  lua_pushstring(ret->L, RIDX_CHANGED_SENSORS);
  lua_newtable(ret->L);
  lua_settable(ret->L, LUA_REGISTRYINDEX);

  // Initialize RIDX_SENSORDEVMAP as a table with weak values that will be
  // used to map lightuserdata to Lua objects
  lua_pushstring(ret->L, RIDX_SENSORDEVMAP);
  lua_newtable(ret->L);
  lua_pushstring(ret->L, "__mode");
  lua_pushstring(ret->L, "v");
  lua_settable(ret->L, -3);
  lua_pushvalue(ret->L, -1);
  lua_setmetatable(ret->L, -2);
  lua_settable(ret->L, LUA_REGISTRYINDEX);

  lua_gc(ret->L, LUA_GCCOLLECT, 0);

  return ret;
}

void TenshiRuntimeDeinit(TenshiRuntimeState s) {
  if (!s) return;

  ActorDestroyAll(s);

  if (s->L) {
    lua_close(s->L);
  }

  free(s);
}

int LoadStudentcode(TenshiRuntimeState s, const char *data, size_t len,
  TenshiActorState *actor_state) {
  if (actor_state) {
    *actor_state = NULL;
  }

  TenshiActorState a = ActorCreate(s);
  if (!a) return LUA_ERRRUN;  // TODO(rqou): Return the correct error?

  int ret = luaL_loadbuffer(a->L, data, len, "<student code>");
  if (ret != LUA_OK) {
    ActorDestroy(a);
    return ret;
  }

  if (actor_state) {
    *actor_state = a;
  }

  return LUA_OK;
}

void print_traceback(lua_State *L) {
  const char *msg = lua_tostring(L, -1);
  if (msg)  /* is error object a string? */
    luaL_traceback(L, L, msg, 0);  /* use standard traceback */
  else if (!lua_isnoneornil(L, -1)) {  /* non-string error object? */
    /* try its 'tostring' metamethod */
    if (!luaL_callmeta(L, -1, "__tostring"))
      lua_pushliteral(L, "(no error message)");
  }  /* else no error object, does nothing */

  const char *err_w_traceback = lua_tostring(L, -1);
  printf("%s\n", err_w_traceback);
}

int TenshiRunQuanta(TenshiRuntimeState s) {
  // Do timeouts
  ActorProcessTimeouts(s);

  // Run the sensor actor
  // Dup the function first, as it disappears when we exit
  // TODO(rqou): Will we have a problem with the hardcoded op limit?
  lua_pushvalue(s->sensor_actor->L, -1);
  int ret = threading_run_ops(s->sensor_actor->L, 1000, NULL);
  if (ret != THREADING_EXITED) {
    printf("There was an error running the sensor actor!\n");
    print_traceback(s->sensor_actor->L);
    return ret;
  }

  // Run the main code
  int ops_left = QUANTA_OPCODES;
  while (ops_left > 0) {
    TenshiActorState a;
    ret = ActorDequeueHead(s, &a);
    if (ret != LUA_OK) return ret;

    if (!a) {
      printf("NOTHING TO RUN!\n");
      break;
    }

    ret = threading_run_ops(a->L, ops_left, &ops_left);
    if (ret == THREADING_ERROR) {
      printf("THERE WAS AN ERROR!\n");
      print_traceback(a->L);

      return LUA_ERRRUN;
    }

    if (ret == THREADING_EXITED) {
      printf("Thread exited!\n");
      ActorDestroy(a);
    } else if (ret == THREADING_YIELD) {
      printf("Thread yielded (blocked)!\n");
      ret = ActorSetBlocked(a);
      if (ret != LUA_OK) return ret;
    } else if (ret == THREADING_PREEMPT) {
      // Requeue it
      printf("Thread preempted!\n");
      ret = ActorSetRunnable(a, 0);
      if (ret != LUA_OK) return ret;
    }
  }

  // Run the actuator actor
  // Dup the function first, as it disappears when we exit
  lua_pushvalue(s->actuator_actor->L, -1);
  ret = threading_run_ops(s->actuator_actor->L, 1000, NULL);
  if (ret != THREADING_EXITED) {
    printf("There was an error running the actuator actor!\n");
    print_traceback(s->actuator_actor->L);
    return ret;
  }

  lua_gc(s->L, LUA_GCCOLLECT, 0);

  return LUA_OK;
}

void TenshiRegisterCFunctions(TenshiRuntimeState s, const luaL_Reg *l) {
  lua_getglobal(s->L, "__runtimeinternal");
  luaL_setfuncs(s->L, l, 0);
  lua_pop(s->L, 1);
}

static int _TenshiFlagSensor(lua_State *L) {
  // stack is dev_raw
  lua_pushstring(L, RIDX_CHANGED_SENSORS);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_pushvalue(L, -2);
  lua_pushboolean(L, 1);
  // stack is dev_raw, changed_sensors, dev_raw, true
  lua_settable(L, -3);
  lua_pop(L, 2);

  return 0;
}

int TenshiFlagSensor(TenshiRuntimeState s, const void *const dev) {
  lua_pushcfunction(s->L, _TenshiFlagSensor);
  lua_pushlightuserdata(s->L, dev);
  return lua_pcall(s->L, 1, 0, 0);
}
