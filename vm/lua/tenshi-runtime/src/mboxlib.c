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

// A mailbox object looks like the following:
// {
//    -- TODO(rqou): Metatable
//    __mbox = {
//      -- Internal mbox state
//      buffer = {} -- circular buffer
//      readidx = <nnn>
//      writeidx = <nnn>
//      count = <nnn>
//      capacity = <nnn>
//      blockedSend = {}  -- Tasks that are blocked and can't send (full)
//      blockedRecv = {}  -- Tasks that are blocked and can't recv (empty)
//    }
//    -- Possibly other fields
// }

#include "inc/mboxlib.h"
#include "inc/actorlib.h"
#include "lauxlib.h"    // NOLINT(build/include)
#include "threading.h"    // NOLINT(build/include)

static int MBoxSend(lua_State *L);
static int MBoxRecv(lua_State *L);

static const luaL_Reg mbox_global_funcs[] = {
  {"send", MBoxSend},
  {"recv", MBoxRecv},
  {NULL, NULL}
};

void tenshi_open_mbox(lua_State *L) {
  // Initialize the functions we load into the global scope.
  lua_pushglobaltable(L);
  luaL_setfuncs(L, mbox_global_funcs, 0);
  lua_pop(L, 1);
}

// mbox_internal = MBoxCreateInternal()
// TODO(rqou): Options? Capacity?
int MBoxCreateInternal(lua_State *L) {
  lua_newtable(L);
  lua_pushstring(L, "buffer");
  lua_newtable(L);
  lua_settable(L, -3);
  lua_pushstring(L, "readidx");
  lua_pushinteger(L, 1);
  lua_settable(L, -3);
  lua_pushstring(L, "writeidx");
  lua_pushinteger(L, 1);
  lua_settable(L, -3);
  lua_pushstring(L, "count");
  lua_pushinteger(L, 0);
  lua_settable(L, -3);
  lua_pushstring(L, "capacity");
  // TODO(rqou): Don't hardcode capacity
  lua_pushinteger(L, 16);
  lua_settable(L, -3);
  lua_pushstring(L, "blockedSend");
  lua_newtable(L);
  lua_settable(L, -3);
  lua_pushstring(L, "blockedRecv");
  lua_newtable(L);
  lua_settable(L, -3);

  return 1;
}

// mbox_public = MBoxCreate(mbox_internal)
int MBoxCreate(lua_State *L) {
  // Stack is mbox_internal
  lua_newtable(L);
  lua_pushstring(L, "__mbox");
  lua_pushvalue(L, -3);
  lua_settable(L, -3);
  // Stack is mbox_internal, mbox_public
  lua_copy(L, -1, -2);
  lua_pop(L, 1);
  // Stack is mbox_public
  // TODO(rqou): Set its metatable
  return 1;
}

// Subroutine for MBoxSendReal. Check if every mailbox has room for one item.
// Returns true (nonzero) if there is room. If there is not enough space, we
// add ourselves to the blocked list for every mailbox that doesn't have enough
// room. Input stack is alternating mailbox and value (only mailbox will be
// read).
// NOT A LUA C FUNCTION.
static int MBoxSendCheckSpace(lua_State *L, int num_mboxes) {
  // In order to handle sending to the same mailbox multiple times, here we
  // total up the number of times each mailbox is referenced. We will then
  // check whether there is enough space to push all of that data into the
  // mailbox at once.

  // Total up references to each mailbox
  // stack is ...args...
  lua_newtable(L);
  for (int i = 0; i < num_mboxes; i++) {
    // stack is ...args..., count_table
    lua_pushstring(L, "__mbox");
    lua_gettable(L, i * 2 + 1);
    lua_pushvalue(L, -1);
    // stack is ...args..., count_table, mboxinternal, mboxinternal
    lua_gettable(L, -3);
    // stack is ...args..., count_table, mboxinternal, curr_count
    // nil will cause tointeger to return 0, which is fine
    int new_count = lua_tointeger(L, -1) + 1;
    lua_pop(L, 1);
    lua_pushinteger(L, new_count);
    // stack is ...args..., count_table, mboxinternal, new_count
    lua_settable(L, -3);
    // stack is ...args..., count_table
  }

  // Check if each mailbox has enough space
  // stack is ...args..., count_table
  int enough_space = 1;
  for (int i = 0; i < num_mboxes; i++) {
    lua_pushstring(L, "__mbox");
    lua_gettable(L, i * 2 + 1);
    lua_pushstring(L, "count");
    lua_gettable(L, -2);
    // stack is ...args..., count_table, mboxinternal, count
    int bufferlen = lua_tointeger(L, -1);
    lua_pop(L, 1);
    // stack is ...args..., count_table, mboxinternal
    lua_pushstring(L, "capacity");
    lua_gettable(L, -2);
    int buffercapacity = lua_tointeger(L, -1);
    lua_pop(L, 1);
    // stack is ...args..., count_table, mboxinternal
    lua_pushvalue(L, -1);
    lua_gettable(L, -3);
    int num_to_send = lua_tointeger(L, -1);
    lua_pop(L, 1);
    // stack is ...args..., count_table, mboxinternal

    if (bufferlen + num_to_send > buffercapacity) {
      // Oh dear, we don't have enough space!
      // TODO(rqou): We can only support blocking at this point.

      enough_space = 0;

      // Add ourselves to the blocked list.
      lua_pushstring(L, "blockedSend");
      lua_gettable(L, -2);
      // stack is ...args..., count_table, mboxinternal, blockedsend
      lua_len(L, -1);
      int num_blocked_send = lua_tointeger(L, -1);
      lua_pop(L, 1);
      lua_pushinteger(L, num_blocked_send + 1);
      lua_pushthread(L);
      lua_settable(L, -3);
      // stack is ...args..., count_table, mboxinternal, blockedsend
      lua_pop(L, 2);

      // We cannot break here because we add ourselves to the blocked list
      // here.
    } else {
      // This is fine, this mailbox had enough space.
      lua_pop(L, 1);
      // stack is ...args..., count_table
    }
  }
  // stack is ...args..., count_table
  lua_pop(L, 1);
  // stack is ...args...

  return enough_space;
}

// Unblock the actor at the top of the stack. The actor internal __mbox
// structure should be on the top of the stack. field_name is either
// blockedRecv or blockedSend.
// NOT A LUA C FUNCTION.
static void MBoxUnblockActor(lua_State *L, const char *field_name) {
  // stack is ..., mboxinternal
  lua_pushstring(L, field_name);
  lua_gettable(L, -2);
  // stack is ..., mboxinternal, blockedsend/recv
  lua_len(L, -1);
  int num_to_unblock = lua_tointeger(L, -1);
  lua_pop(L, 1);

  for (int i = 0; i < num_to_unblock; i++) {
    lua_pushcfunction(L, ActorFindInTaskset);
    lua_pushinteger(L, i + 1);
    lua_gettable(L, -3);
    // stack is ..., mboxinternal, blockedsend/recv, findintaskset, thread
    lua_call(L, 1, 1);
    // Check if thread was found. If not, must have exited via some other
    // means. That should be OK.
    // stack is ..., mboxinternal, blockedsend/recv, actorstate
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
    } else {
      TenshiActorState a = ActorObjectGetCState(L);
      lua_pop(L, 1);
      // stack is ..., mboxinternal, blockedsend/recv
      if (ActorSetUnblocked(a) != LUA_OK) {
        lua_pushstring(L, "could not unblock actor!");
        lua_error(L);
      }
    }
  }

  lua_pop(L, 1);
  // stack is ..., mboxinternal
}

static int MBoxSendReal(lua_State *L, int status, int ctx) {
  // Called either on initial attempt to send or when we tried, failed,
  // yielded, and came back.

  // TODO(rqou): Handle options

  // Yes, the flooring division is intentional and ok here. If we get an odd
  // number of arguments, the last one is interpreted as options.
  int num_mboxes = lua_gettop(L) / 2;

  // Check if all mailboxes have space
  // stack is ...args...
  if (!MBoxSendCheckSpace(L, num_mboxes)) {
    // Yield ourselves to the scheduler. We will be rerun (and all mailboxes
    // will be rescanned) if somebody receives data from any of our mailboxes.
    // TODO(rqou): This is not the most efficient way to do this. We might
    // want to eventually track all of the mailboxes that have to clear before
    // we even consider rescanning.
    lua_pushinteger(L, THREADING_YIELD);
    return lua_yieldk(L, 1, 0, MBoxSendReal);
  }

  // We know we have enough space to send all the data.
  // stack is ...args...
  for (int i = 0; i < num_mboxes; i++) {
    lua_pushstring(L, "__mbox");
    // Read __mbox from the i'th mailbox (args alternate mailbox, value)
    lua_gettable(L, i * 2 + 1);
    // stack is ...args..., mboxinternal
    lua_pushstring(L, "buffer");
    lua_gettable(L, -2);
    // stack is ...args..., mboxinternal, buffer
    lua_pushstring(L, "writeidx");
    lua_gettable(L, -3);
    int writeidx = lua_tointeger(L, -1);
    // Read the i'th value (args alternate mailbox, value)
    lua_pushvalue(L, i * 2 + 2);
    // stack is ...args..., mboxinternal, buffer, writeidx, sendval
    lua_settable(L, -3);
    lua_pop(L, 1);
    // stack is ...args..., mboxinternal

    // Need to increment count and writeidx
    lua_pushstring(L, "capacity");
    lua_gettable(L, -2);
    int capacity = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "writeidx");
    lua_pushinteger(L, (writeidx % capacity) + 1);
    lua_settable(L, -3);

    lua_pushstring(L, "count");
    lua_gettable(L, -2);
    int count_old = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "count");
    lua_pushinteger(L, count_old + 1);
    lua_settable(L, -3);

    lua_pop(L, 1);
    // stack is ...args...
  }

  // All data has been sent. We now need to unblock all the actors.
  for (int i = 0; i < num_mboxes; i++) {
    lua_pushstring(L, "__mbox");
    // Read __mbox from the i'th mailbox (args alternate mailbox, value)
    lua_gettable(L, i * 2 + 1);

    MBoxUnblockActor(L, "blockedRecv");
    lua_pop(L, 1);
  }

  return 0;
}

// send(mailbox0, value0, ..., mailboxn, valuen, [options])
int MBoxSend(lua_State *L) {
  return MBoxSendReal(L, 0, 0);
}

static int MBoxRecvReal(lua_State *L, int status, int ctx) {
  // Called either on initial attempt to recv or when we tried, failed,
  // yielded, and came back.

  // TODO(rqou): Handle options. This line will need to change when options
  // are added.
  int num_mboxes = lua_gettop(L);

  int have_data = 0;

  // Check if any mailboxes have data
  for (int i = 0; i < num_mboxes; i++) {
    lua_pushstring(L, "__mbox");
    lua_gettable(L, i + 1);
    lua_pushstring(L, "count");
    lua_gettable(L, -2);
    // stack is ...args..., mboxinternal, count
    int bufferlen = lua_tointeger(L, -1);
    lua_pop(L, 2);
    // stack is ...args...

    if (bufferlen > 0) {
      // We do have data!
      have_data = 1;

      // Pull the data out of it
      lua_pushstring(L, "__mbox");
      lua_gettable(L, i + 1);
      // stack is ...args..., mboxinternal
      lua_pushstring(L, "buffer");
      lua_gettable(L, -2);
      // stack is ...args..., mboxinternal, buffer
      lua_pushstring(L, "readidx");
      lua_gettable(L, -3);
      int readidx = lua_tointeger(L, -1);
      // stack is ...args..., mboxinternal, buffer, readidx
      lua_gettable(L, -2);
      // stack is ...args..., mboxinternal, buffer, val
      lua_insert(L, 1);
      // stack is val, ...args..., mboxinternal, buffer
      lua_pop(L, 1);
      // stack is val, ...args..., mboxinternal

      // Need to decrement count and increment readidx
      lua_pushstring(L, "capacity");
      lua_gettable(L, -2);
      int capacity = lua_tointeger(L, -1);
      lua_pop(L, 1);
      lua_pushstring(L, "readidx");
      lua_pushinteger(L, (readidx % capacity) + 1);
      lua_settable(L, -3);

      lua_pushstring(L, "count");
      lua_pushinteger(L, bufferlen - 1);
      lua_settable(L, -3);
      // stack is val, ...args..., mboxinternal

      // We read some data. We now need to unblock all the actors for this one
      // mailbox.
      MBoxUnblockActor(L, "blockedSend");
      // stack is val, ...args..., mboxinternal
      lua_pop(L, 1 + num_mboxes);
      // stack is val
      // We found our 1 element to return
      break;
    }
  }

  if (!have_data) {
    // Add ourselves to the blocked list for every mailbox.
    for (int i = 0; i < num_mboxes; i++) {
      lua_pushstring(L, "__mbox");
      lua_gettable(L, i + 1);
      // stack is ...args..., mboxinternal

      lua_pushstring(L, "blockedRecv");
      lua_gettable(L, -2);
      // stack is ...args..., mboxinternal, blockedrecv
      lua_len(L, -1);
      int num_blocked_recv = lua_tointeger(L, -1);
      lua_pop(L, 1);
      lua_pushinteger(L, num_blocked_recv + 1);
      lua_pushthread(L);
      lua_settable(L, -3);
      // stack is ...args..., mboxinternal, blockedrecv
      lua_pop(L, 2);
    }
    // stack is ...args...
    // Yield ourselves to the scheduler. We will be rerun (and all mailboxes
    // will be rescanned) if somebody sends data to any of our mailboxes.
    // TODO(rqou): This is not the most efficient way to do this. We might
    // want to eventually track all of the mailboxes that have to fill before
    // we even consider rescanning.
    lua_pushinteger(L, THREADING_YIELD);
    return lua_yieldk(L, 1, 0, MBoxRecvReal);
  } else {
    // val was put on stack above
    return 1;
  }
}

// value = recv(mailbox0, ..., mailboxn, [options])
// The value that will be returned will be either a value from the leftmost
// mailbox or a value from the earliest one to arrive that is also leftmost.
// TODO(rqou): Improve this explanation.
int MBoxRecv(lua_State *L) {
  return MBoxRecvReal(L, 0, 0);
}
