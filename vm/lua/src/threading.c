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

#include <threading.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lstate.h>

/*
 * This module implements utilities used to implement non-cooperative threads / 
 * Actors. See the header for further documentation.
 */

static lua_Hook threading_old_hook = NULL;

void threading_yield_hook(lua_State *L, lua_Debug *ar) {
  if (ar->event & LUA_HOOKCOUNT) {
    lua_yield(L, 0);
  } else if (threading_old_hook != NULL) {
    (*threading_old_hook)(L, ar);
  }
}

void threading_setup(lua_State *L) {
  threading_old_hook = lua_gethook(L);
}

int threading_run_ops(lua_State *L, int op_count, int *ops_left) {
  lua_sethook(L, &threading_yield_hook, lua_gethookmask(L) | LUA_MASKCOUNT, 
      op_count);
  // The Lua hooking functionality uses this flag in what appears to be a
  // workaround for how luaV_execute functions. It normally sets the hookcount
  // to 1 so that luaV_execute will immediately reenter the hook logic.
  // It seems to use this to reset the hookcount. However, it would not make
  // sense to call the user hook function again, as the interpreter would have
  // run for 0 ticks. Lua therefore uses this flag in what appears to be a
  // workaround so that the user hook won't be called that first time. However,
  // when we call lua_sethook, it already resets hookcount. If this flag is
  // not cleared, the interpreter will actually run 2x the number of opcodes.
  // So as an apparent hack on top of a hack, we clear this flag here.
  L->ci->callstatus &= ~CIST_HOOKYIELD;
  
  int old_tos = lua_gettop(L);
  // Subtract 1 to ignore the argument to lua_resume if first run
  // TODO(rqou): This is really hacky. From reading the comments in ldo.c, it
  // appears that this works unless there is an error (resuming a coroutine
  // that is already running).
  if (L->status == LUA_OK) {
    old_tos--;
  }
  int status = lua_resume(L, 0, 0);
  int new_tos = lua_gettop(L);

  if (ops_left) {
    int left = L->hookcount;
    // TODO(rqou): For reasons that I don't understand, luaG_traceexec sets
    // hookcount back to 1 when it is 0. We set it back to 0 here. This has a
    // side-effect where any thread that yields to block with exactly 1 opcode
    // left will "lose" this one cycle. It probably doesn't matter though.
    if (left == 1) left = 0;
    *ops_left = left;
  }

  if (status == LUA_OK) {
    return THREADING_EXITED;
  } else if (status == LUA_YIELD) {
    // It is impossible to return data from lua_yield with in a hook for some
    // reason (luaD_hook calls restorestack). Therefore, we assume we are
    // preempted if we have no information pushed. Otherwise, if we are
    // yielding due to blocking on something, the call to yield must have
    // pushed an int indicating why it yielded.
    if (old_tos == new_tos) {
      return THREADING_PREEMPT;
    } else {
      int ret = lua_tointeger(L, -1);
      lua_pop(L, 1);
      return ret;
    }
  } else {
    return THREADING_ERROR;
  }
}
