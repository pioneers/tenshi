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

#ifndef THREADING_H_
#define THREADING_H_

#include <lua.h>
/*
 * This module implements utilities used to implement non-cooperative threads / 
 * Actors. It relies on using lua_sethook(), so calls to lua_sethook() must be 
 * careful to stil call the lua_Hook registered by threading_run_ops(), or this 
 * module will stop working.
 */

/*
 * Must be called on the root lua_State before any threads are run.
 * Before this function is called, no other functions in this module will work 
 * as described. Interferes with any other lua_Hook from being called due to 
 * the count option (see lua_sethook() in the Lua manual for details).
 */
void threading_setup(lua_State *L);

// The thread/actor has returned and finished execution
#define THREADING_EXITED  0
// The thread/actor has been preempted by this module's hook.
#define THREADING_PREEMPT 1
// The thread/actor has yielded due to being blocked.
#define THREADING_YIELD   2
// An error occurred while running the thread/actor
#define THREADING_ERROR   3

/*
 * Runs a lua_State for some number of opcodes Returns one of the above ints.
 * If ops_left is not NULL, store the number of opcodes left that haven't been
 * run (will be nonzero if thread yields to block on something).
 */
int threading_run_ops(lua_State *L, int op_count, int *ops_left);

#endif  // THREADING_H_
