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

#ifndef INC_ACTOR_SCHED_H_
#define INC_ACTOR_SCHED_H_

typedef struct _TenshiActorState* TenshiActorState;

#include "inc/actorlib.h"
#include "inc/runtime_entry.h"
#include "lua.h"  // NOLINT(build/include)

#define RIDX_ALLACTORS    "tenshi.allActors"
#define RIDX_RUNQUEUELO   "tenshi.runQueueLo"
#define RIDX_RUNQUEUEHI   "tenshi.runQueueHi"

// To be called in protected mode.
extern int ActorSchedulerInit(lua_State *L);

// Returns NULL on error. Automatically registers the actor with the scheduler.
extern TenshiActorState ActorCreate(TenshiRuntimeState s);
extern void ActorDestroy(TenshiActorState a);

// Returns LUA_OK on success
extern int ActorSetRunnable(TenshiActorState a, int highPriority);

// Returns LUA_OK on success.
extern int ActorDequeueHead(TenshiRuntimeState s, TenshiActorState *a_out);

// Close all actors, used for shutting down VM
extern void ActorDestroyAll(TenshiRuntimeState s);

// To be called in protected mode. Takes one argument, the thread to look up,
// and returns one argument, the found actor object.
extern int ActorFindInTaskset(lua_State *L);

#endif  // INC_ACTOR_SCHED_H_
