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

#include "inc/actor_sched.h"
#include "inc/runtime_internal.h"

int ActorSchedulerInit(lua_State *L) {
  // This scheduler is the most naive possible scheduler consisting of one
  // priority level with a FIFO of tasks ready to run. Scheduler state is
  // stored in the Lua registry.

  // Initialize the task set with an empty table. The task set will eventually
  // contain a table with keys being the Lua thread objects and values being
  // lightuserdata pointing to the actor data in C.
  lua_pushstring(L, RIDX_ALLACTORS);
  lua_newtable(L);
  lua_settable(L, LUA_REGISTRYINDEX);

  // The runnable queue is a singly-linked list. It does not need to be
  // initialized here because it will be nil when nothing is available to run.

  return 0;
}

// Called in protected mode
static int ActorAddToTaskset(lua_State *L) {
  lua_pushstring(L, RIDX_ALLACTORS);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_pushthread(L);
  lua_pushvalue(L, 1);
  lua_settable(L, -3);
  lua_pop(L, 1);

  return 0;
}

TenshiActorState ActorCreate(TenshiRuntimeState s) {
  TenshiActorState ret =
    (TenshiActorState)(malloc(sizeof(struct _TenshiActorState)));

  if (!ret) return NULL;

  ret->s = s;
  ret->L = lua_newthread(s->L);
  if (!ret->L) {
    ActorDestroy(ret);
    return NULL;
  }

  // Add this to the task set
  lua_pushcfunction(ret->L, ActorAddToTaskset);
  lua_pushlightuserdata(ret->L, ret);
  if (lua_pcall(ret->L, 1, 0, 0) != LUA_OK) {
    ActorDestroy(ret);
    return NULL;
  }

  return ret;
}

// Called in protected mode
static int ActorRemoveFromTaskset(lua_State *L) {
  lua_pushstring(L, RIDX_ALLACTORS);
  lua_gettable(L, LUA_REGISTRYINDEX);
  lua_pushthread(L);
  lua_pushnil(L);
  lua_settable(L, -3);
  lua_pop(L, 1);

  return 0;
}

void ActorDestroy(TenshiActorState a) {
  if (!a) return;

  if (a->L) {
    // Remove this from the task set (by setting its value to nil)
    lua_pushcfunction(a->L, ActorRemoveFromTaskset);
    lua_pcall(a->L, 0, 0, 0);
  }

  free(a);
}

// Called in protected mode
static int _ActorDestroyAll(lua_State *L) {
  lua_pushstring(L, RIDX_ALLACTORS);
  lua_gettable(L, LUA_REGISTRYINDEX);

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    // Free the native TenshiActorState (the value)
    free(lua_topointer(L, -1));
    // We have to leave the key, so duplicate it
    lua_pushvalue(L, -2);
    lua_pushnil(L);
    // Stack is now table, key, value, key, nil
    lua_settable(L, -5);
    // Pop value
    lua_pop(L, 1);
    // Stack is now table, key
  }

  lua_pop(L, 1);
  return 0;
}

void ActorDestroyAll(TenshiRuntimeState s) {
  if (!s) return;

  if (s->L) {
    lua_pushcfunction(s->L, _ActorDestroyAll);
    lua_pcall(s->L, 0, 0, 0);
  }

  // TODO(rqou): Runnable queue?
}
