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

#ifndef INC_MBOXLIB_H_
#define INC_MBOXLIB_H_

#include <stdint.h>
#include <stdlib.h>
#include "inc/runtime_entry.h"
#include "lua.h"  // NOLINT(build/include)
#include "lauxlib.h"    // NOLINT(build/include)

// Key used to store the sensor and actuator mailboxes table
#define RIDX_MBOXLIB_SENSORS_ACTUATORS    "tenshi.mboxlib.sensors_actuators"
// Key used to store the changed actuator table
#define RIDX_MBOXLIB_CHANGED_TABLE        "tenshi.mboxlib.changed_actuators"
// Key used to store the mailbox metatable in the Lua registry
#define RIDX_MBOXLIB_METATABLE            "tenshi.mboxlib.metatable"

// This is used as a somewhat hacky way to get mailbox functions into e.g.
// actors rather than actually handling inheritance using more metatables.
extern const luaL_Reg mbox_metatable_funcs[];

extern void tenshi_open_mbox(lua_State *L);

// Creates a new mailbox internal state (not associated with anything)
// and leaves it at the top of the stack. To be called in protected mode.
extern int MBoxCreateInternal(lua_State *L);

// Creates a new public, Lua-facing mailbox wrapping the internal mailbox
// state on the stack and returns the new mailbox wrapper. To be called in
// protected mode.
extern int MBoxCreate(lua_State *L);

// Create a new mailbox (both internal state and Lua object) with the passed-in
// ID. This mailbox will be specifically flagged as an "Actuator" mailbox,
// which currently means that it will be stored in a global table and will
// be flagged specially when data is sent to it (for the purpose of checking
// what needs to be updated). The mailbox is NOT left on the stack.
// Returns LUA_OK on success.
extern int MBoxCreateActuator(TenshiRuntimeState s,
  const uint8_t *id, size_t id_len);

// Create a new mailbox (both internal state and Lua object) with the passed-in
// ID. This mailbox will be specifically flagged as an "Sensor" mailbox,
// which currently means that it will be stored in a global table. The
// mailbox is NOT left on the stack.
// Returns LUA_OK on success.
extern int MBoxCreateSensor(TenshiRuntimeState s,
  const uint8_t *id, size_t id_len);

// Send the data on the top of the stack to the sensor mailbox with the
// passed-in ID. Pops the data. Returns LUA_OK on success.
extern int MBoxSendSensor(TenshiRuntimeState s,
  const uint8_t *id, size_t id_len);

// Receive an element from the actuator mailbox with the passed-in ID.
// Pushes the data onto the top of the stack. Returns LUA_OK on success.
extern int MBoxRecvActuator(TenshiRuntimeState s,
  const uint8_t *id, size_t id_len);

typedef struct update_info {
  char *id;
  size_t id_len;
  size_t num_data;
  struct update_info *next;
} update_info;

// Returns a linked list of the updated actuators and then clears the internal
// list.
extern update_info *MBoxGetActuatorsChanged(TenshiRuntimeState s);

extern void MBoxFreeUpdateInfo(update_info *i);

#endif  // INC_MBOXLIB_H_
