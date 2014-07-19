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
#include "lua.h"        // NOLINT(build/include)
#include "lualib.h"     // NOLINT(build/include)
#include "lauxlib.h"    // NOLINT(build/include)
#include "lbaselib.h"   // NOLINT(build/include)
#include "lstrlib.h"    // NOLINT(build/include)
#include "threading.h"  // NOLINT(build/include)
#include "../actuators.lua.h"   // NOLINT(build/include)
#include "../game.lua.h"        // NOLINT(build/include)
#include "../get_device.lua.h"  // NOLINT(build/include)
#include "../pieles.lua.h"      // NOLINT(build/include)
#include "../signals.lua.h"     // NOLINT(build/include)
#include "../triggers.lua.h"    // NOLINT(build/include)
#include "../units.lua.h"       // NOLINT(build/include)

// Custom version of baselib with some functions omitted
static const luaL_Reg tenshi_base_funcs[] = {
  {"assert", luaB_assert},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"next", luaB_next},
  {"pairs", luaB_pairs},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"xpcall", luaB_xpcall},
  {NULL, NULL}
};

static int tenshi_open_base(lua_State *L) {
  /* set global _G */
  lua_pushglobaltable(L);
  lua_pushglobaltable(L);
  lua_setfield(L, -2, "_G");
  /* open lib into global table */
  luaL_setfuncs(L, tenshi_base_funcs, 0);
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");  /* set global _VERSION */
  return 1;
}

// Custom version of strlib with some functions (dump) omitted
static const luaL_Reg tenshi_strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"find", str_find},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_lower},
  {"match", str_match},
  {"rep", str_rep},
  {"reverse", str_reverse},
  {"sub", str_sub},
  {"upper", str_upper},
  {"dumpfloat", dumpfloat_l},
  {"dumpint", dumpint_l},
  {"undumpfloat", undumpfloat_l},
  {"undumpint", undumpint_l},
  {NULL, NULL}
};

static int tenshi_open_string(lua_State *L) {
  luaL_newlib(L, tenshi_strlib);
  lua_createtable(L, 0, 1);  /* table to be metatable for strings */
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_pop(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_pop(L, 1);  /* pop metatable */
  return 1;
}

static int tenshi_open_units(lua_State *L) {
  luaL_loadbuffer(L, units_lua, sizeof(units_lua), "units.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_signals(lua_State *L) {
  luaL_loadbuffer(L, signals_lua, sizeof(signals_lua), "signals.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_actuators(lua_State *L) {
  luaL_loadbuffer(L, actuators_lua, sizeof(actuators_lua), "actuators.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_triggers(lua_State *L) {
  luaL_loadbuffer(L, triggers_lua, sizeof(triggers_lua), "triggers.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_pieles(lua_State *L) {
  luaL_loadbuffer(L, pieles_lua, sizeof(pieles_lua), "pieles.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_game(lua_State *L) {
  luaL_loadbuffer(L, game_lua, sizeof(game_lua), "game.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

// Tenshi modules (omits some Lua modules, adds some new modules)
static const luaL_Reg tenshi_loadedlibs[] = {
  {"_G", tenshi_open_base},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_STRLIBNAME, tenshi_open_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {"units", tenshi_open_units},
  {"__signals", tenshi_open_signals},
  {"__actuators", tenshi_open_actuators},
  {"triggers", tenshi_open_triggers},
  {"pieles", tenshi_open_pieles},
  {"game", tenshi_open_game},
  {NULL, NULL}
};

// We specifically expose only a subset of the normal Lua standard library.
// This function loads only the functions we want.
static void TenshiRuntime_openlibs(lua_State *L) {
  const luaL_Reg *lib;
  for (lib = tenshi_loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}

static int tenshi_open_get_device(lua_State *L) {
  luaL_loadbuffer(L, get_device_lua, sizeof(get_device_lua), "get_device.lua");
  lua_pcall(L, 0, 0, 0);
  return 0;
}

TenshiRuntimeState TenshiRuntimeInit(void) {
  TenshiRuntimeState ret;

  ret = (TenshiRuntimeState)(malloc(sizeof(struct _TenshiRuntimeState)));
  if (!ret) return NULL;

  ret->L = luaL_newstate();
  if (!ret->L) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Load libraries
  TenshiRuntime_openlibs(ret->L);

  // Load actor library. This is special because it loads into the global
  // scope and not into a specific module.
  tenshi_open_actor(ret->L);
  // Load mailbox library. Similar thing.
  tenshi_open_mbox(ret->L);
  // Load get_device implementation.
  tenshi_open_get_device(ret->L);

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

int TenshiRunQuanta(TenshiRuntimeState s) {
  int ops_left = QUANTA_OPCODES;

  while (ops_left > 0) {
    TenshiActorState a;
    int ret = ActorDequeueHead(s, &a);
    if (ret != LUA_OK) return ret;

    if (!a) {
      printf("NOTHING TO RUN!\n");
      return LUA_OK;
    }

    ret = threading_run_ops(a->L, ops_left, &ops_left);
    if (ret == THREADING_ERROR) {
      printf("THERE WAS AN ERROR!\n");

      const char *msg = lua_tostring(a->L, -1);
      if (msg)  /* is error object a string? */
        luaL_traceback(a->L, a->L, msg, 0);  /* use standard traceback */
      else if (!lua_isnoneornil(a->L, -1)) {  /* non-string error object? */
        /* try its 'tostring' metamethod */
        if (!luaL_callmeta(a->L, -1, "__tostring"))
          lua_pushliteral(a->L, "(no error message)");
      }  /* else no error object, does nothing */

      const char *err_w_traceback = lua_tostring(a->L, -1);
      printf("%s\n", err_w_traceback);
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

  return LUA_OK;
}

void TenshiMainStackPushInt(TenshiRuntimeState s, lua_Integer i) {
  lua_pushinteger(s->L, i);
}
void TenshiMainStackPushUInt(TenshiRuntimeState s, lua_Unsigned i) {
  lua_pushunsigned(s->L, i);
}
void TenshiMainStackPushFloat(TenshiRuntimeState s, lua_Number i) {
  lua_pushnumber(s->L, i);
}
lua_Integer TenshiMainStackGetInt(TenshiRuntimeState s) {
  lua_Integer ret = lua_tointeger(s->L, -1);
  lua_pop(s->L, 1);
  return ret;
}
lua_Unsigned TenshiMainStackGetUInt(TenshiRuntimeState s) {
  lua_Unsigned ret = lua_tounsigned(s->L, -1);
  lua_pop(s->L, 1);
  return ret;
}
lua_Number TenshiMainStackGetFloat(TenshiRuntimeState s) {
  lua_Number ret = lua_tonumber(s->L, -1);
  lua_pop(s->L, 1);
  return ret;
}
