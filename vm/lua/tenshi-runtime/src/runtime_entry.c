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
#include "inc/runtime_internal.h"
#include "lua.h"        // NOLINT(build/include)
#include "lualib.h"     // NOLINT(build/include)
#include "lauxlib.h"    // NOLINT(build/include)
#include "lbaselib.h"   // NOLINT(build/include)
#include "lstrlib.h"    // NOLINT(build/include)
#include "threading.h"  // NOLINT(build/include)

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

// Tenshi modules (omits some Lua modules, adds some new modules)
static const luaL_Reg tenshi_loadedlibs[] = {
  {"_G", tenshi_open_base},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_STRLIBNAME, tenshi_open_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
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
    if (ret == THREADING_ERROR) return LUA_ERRRUN;

    if (ret == THREADING_EXITED) {
      printf("Thread exited!\n");
      ActorDestroy(a);
    } else if (ret == THREADING_YIELD) {
      printf("ERROR: Yield to block not implemented!\n");
      return LUA_ERRRUN;
    } else if (ret == THREADING_PREEMPT) {
      // Requeue it
      printf("Thread preempted!\n");
      ret = ActorSetRunnable(a, 0);
      if (ret != LUA_OK) return ret;
    }
  }

  return LUA_OK;
}
