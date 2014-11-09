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

#ifndef INC_RUNTIME_INTERNAL_H_
#define INC_RUNTIME_INTERNAL_H_

struct _TenshiRuntimeState {
  // Lua state object -- refers to "main" thread (that never executes)
  lua_State *L;
  // Thread called before "main" part of loop that translates updated
  // sensors into data sent into mailboxes
  TenshiActorState sensor_actor;
  // Thread called after "main" part of loop that translates data sent into
  // mailboxes into actuator update functions
  TenshiActorState actuator_actor;
};

struct _TenshiActorState {
  // Main runtime data structure pointer
  TenshiRuntimeState s;
  // This actor's lua_State
  lua_State *L;
  // Set to true if this actor is blocked. Prevents multiple unblocking.
  int isblocked;
  // Set to true if this actor woke because of a timeout.
  int woke_timeout;
  // Used for scheduler linked lists (run queue)
  TenshiActorState next;
};

#endif  // INC_RUNTIME_INTERNAL_H_
