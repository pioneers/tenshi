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

#include "inc/runtime_libs.h"

#include "lualib.h"     // NOLINT(build/include)
#include "lauxlib.h"    // NOLINT(build/include)
#include "lbaselib.h"   // NOLINT(build/include)
#include "lstrlib.h"    // NOLINT(build/include)
#include "../actuators.lc.h"        // NOLINT(build/include)
#include "../game.lc.h"             // NOLINT(build/include)
#include "../pieles.lc.h"           // NOLINT(build/include)
#include "../triggers.lc.h"         // NOLINT(build/include)
#include "../ubjson.lc.h"           // NOLINT(build/include)
#include "../units.lc.h"            // NOLINT(build/include)

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
  luaL_loadbuffer(L, units_lc, sizeof(units_lc), "units.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_actuators(lua_State *L) {
  luaL_loadbuffer(L, actuators_lc, sizeof(actuators_lc), "actuators.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_triggers(lua_State *L) {
  luaL_loadbuffer(L, triggers_lc, sizeof(triggers_lc), "triggers.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_pieles(lua_State *L) {
  luaL_loadbuffer(L, pieles_lc, sizeof(pieles_lc), "pieles.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_game(lua_State *L) {
  luaL_loadbuffer(L, game_lc, sizeof(game_lc), "game.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int tenshi_open_ubjson(lua_State *L) {
  luaL_loadbuffer(L, ubjson_lc, sizeof(ubjson_lc), "ubjson.lua");
  lua_pcall(L, 0, LUA_MULTRET, 0);
  return 1;
}

static int get_registry(lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  return 1;
}

static luaL_Reg tenshi_runtimeinternal[] = {
  {"get_registry", get_registry},
  {NULL, NULL}
};

// Opens part of the __runtimeinternal library. This will be later extended
// by the external user of the runtime by calling TenshiRegisterCFunctions.
static int tenshi_open_runtimeinternal(lua_State *L) {
  luaL_newlib(L, tenshi_runtimeinternal);
  return 1;
}

// Tenshi modules (omits some Lua modules, adds some new modules)
static const luaL_Reg tenshi_loadedlibs_phase1[] = {
  {"_G", tenshi_open_base},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_STRLIBNAME, tenshi_open_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {"units", tenshi_open_units},
  {"__runtimeinternal", tenshi_open_runtimeinternal},
  {NULL, NULL}
};

// We specifically expose only a subset of the normal Lua standard library.
// This function loads only the functions we want.
void TenshiRuntime_openlibs_phase1(lua_State *L) {
  const luaL_Reg *lib;
  for (lib = tenshi_loadedlibs_phase1; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
    lua_gc(L, LUA_GCCOLLECT, 0);
  }
}

// Phase2 libraries are libraries that depend on native code functionality
// like actors/mailboxes
static const luaL_Reg tenshi_loadedlibs_phase2[] = {
  {"__actuators", tenshi_open_actuators},
  {"triggers", tenshi_open_triggers},
  {"pieles", tenshi_open_pieles},
  {"game", tenshi_open_game},
  {"ubjson", tenshi_open_ubjson},
  {NULL, NULL}
};

void TenshiRuntime_openlibs_phase2(lua_State *L) {
  const luaL_Reg *lib;
  for (lib = tenshi_loadedlibs_phase2; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}
