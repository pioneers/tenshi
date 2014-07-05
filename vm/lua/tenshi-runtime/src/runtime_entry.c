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
#include "inc/runtime_internal.h"
#include "lua.h"        // NOLINT(build/include)
#include "lauxlib.h"    // NOLINT(build/include)
#include "threading.h"  // NOLINT(build/include)

TenshiRuntimeState TenshiRuntimeInit(void) {
  TenshiRuntimeState ret;

  ret = (TenshiRuntimeState)(malloc(sizeof(struct _TenshiRuntimeState)));
  if (!ret) return NULL;

  ret->L = luaL_newstate();
  if (!ret->L) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Set up actor scheduler
  lua_pushcfunction(ret->L, ActorSchedulerInit);
  if (lua_pcall(ret->L, 0, 0, 0) != LUA_OK) {
    TenshiRuntimeDeinit(ret);
    return NULL;
  }

  // Set up preemption trick
  threading_setup(ret->L);

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
      ret = ActorSetRunnable(a);
      if (ret != LUA_OK) return ret;
    }
  }

  return LUA_OK;
}
