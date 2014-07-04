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
#include "lua.h"      // NOLINT(build/include)
#include "lauxlib.h"  // NOLINT(build/include)

struct _TenshiRuntimeState {
  lua_State *L;
};

TenshiRuntimeState TenshiRuntimeInit(void) {
  TenshiRuntimeState ret;

  ret = (TenshiRuntimeState)(malloc(sizeof(struct _TenshiRuntimeState)));
  if (!ret) return NULL;

  ret->L = luaL_newstate();
  if (!ret->L) return NULL;

  return ret;
}

void TenshiRuntimeDeinit(TenshiRuntimeState s) {
  if (!s) return;

  if (!s->L) return;
  lua_close(s->L);
}
