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

// An actor object looks like the following:
// {
//    <metatable> = <tenshi.actorlib.metatable>
//    __cstate = <TenshiActorState>
//    __mbox = <Mailbox internal state>
//    -- TODO(rqou): More objects later?
// }

#include "inc/actorlib.h"
#include "inc/mboxlib.h"
#include "inc/runtime_entry.h"
#include "inc/runtime_internal.h"
#include "lauxlib.h"    // NOLINT(build/include)
#include "threading.h"    // NOLINT(build/include)

static int ActorGetOwnActor(lua_State *L);
static int ActorStop(lua_State *L);
static int ActorStart(lua_State *L);

static const luaL_Reg actor_global_funcs[] = {
  {"get_own_actor", ActorGetOwnActor},
  {"start_actor", ActorStart},
  {NULL, NULL}
};

static const luaL_Reg actor_metatable_funcs[] = {
  {"stop", ActorStop},
  {NULL, NULL}
};

void tenshi_open_actor(lua_State *L) {
  // Initialize the actor object metatable (contains implementations of the
  // actor object API)
  lua_pushstring(L, RIDX_ACTORLIB_METATABLE);
  lua_newtable(L);
  luaL_setfuncs(L, actor_metatable_funcs, 0);
  // Also load mailbox metatable functions. We can do this because actors
  // also contain a __mbox field.
  luaL_setfuncs(L, mbox_metatable_funcs, 0);
  // Set __index to this table
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);
  lua_settable(L, LUA_REGISTRYINDEX);

  // Initialize the functions we load into the global scope.
  lua_pushglobaltable(L);
  luaL_setfuncs(L, actor_global_funcs, 0);
  lua_pop(L, 1);
}

// Called in protected mode.
static int _ActorObjectCreate(lua_State *L) {
  // Stack is lightuserdata
  lua_newtable(L);
  lua_pushstring(L, "__cstate");
  lua_pushvalue(L, -3);
  lua_settable(L, -3);
  // Stack is lightuserdata, newtable
  lua_copy(L, -1, -2);
  lua_pop(L, 1);
  // Stack is newtable

  // Create associated mailbox
  lua_pushstring(L, "__mbox");
  lua_pushcfunction(L, MBoxCreateInternal);
  lua_pcall(L, 0, 1, 0);
  lua_settable(L, -3);

  // Set its metatable
  lua_pushstring(L, RIDX_ACTORLIB_METATABLE);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_setmetatable(L, -2);
  return 1;
}

int ActorObjectCreate(lua_State *L, TenshiActorState a) {
  lua_pushcfunction(L, _ActorObjectCreate);
  lua_pushlightuserdata(L, a);
  return lua_pcall(L, 1, 1, 0);
}

TenshiActorState ActorObjectGetCState(lua_State *L) {
  lua_pushstring(L, "__cstate");
  lua_rawget(L, -2);
  TenshiActorState ret = (TenshiActorState)lua_topointer(L, -1);
  lua_pop(L, 1);

  return ret;
}

// actor_obj = get_own_actor()
int ActorGetOwnActor(lua_State *L) {
  lua_pushcfunction(L, ActorFindInTaskset);
  lua_pushthread(L);
  lua_call(L, 1, 1);

  return 1;
}

// actor:stop()
int ActorStop(lua_State *L) {
  // If we are executing, we are definitely not in any run queues. We will only
  // be in the global actor map. We just need to yield THREADING_EXITED and
  // we will be automagically destroyed by the scheduler.
  lua_pushinteger(L, THREADING_EXITED);
  return lua_yield(L, 1);
}

// actor_obj = start_actor(callable)
int ActorStart(lua_State *L) {
  // Get the global state
  lua_pushstring(L, RIDX_RUNTIMESTATE);
  lua_gettable(L, LUA_REGISTRYINDEX);
  TenshiRuntimeState s = (TenshiRuntimeState)lua_topointer(L, -1);
  lua_pop(L, 1);

  // Create an actor
  // TODO(rqou): Hope calling ActorCreate doesn't somehow introduce a subtle
  // bug of some sort.
  TenshiActorState a = ActorCreate(s);
  if (!a) {
    lua_pushstring(L, "could not create new actor object!");
    lua_error(L);
  }

  // Move our callable onto the new actor state.
  lua_xmove(L, a->L, 1);

  // Append the new callable to the run queue
  ActorSetRunnable(a, 0);

  // our stack is empty
  // The object we want can be looked up in the global taskset.
  // TODO(rqou): This is not the most efficient.
  lua_pushcfunction(L, ActorFindInTaskset);
  // TODO(rqou): Is there a better way than doing this and then transplanting
  // the data back?
  lua_pushthread(a->L);
  lua_xmove(a->L, L, 1);
  lua_call(L, 1, 1);

  // stack is new actor object
  return 1;
}
