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

#ifndef INC_RUNTIME_LIBS_H_
#define INC_RUNTIME_LIBS_H_

#include "lua.h"        // NOLINT(build/include)

extern void TenshiRuntime_openlibs_phase1(lua_State *L);
extern void TenshiRuntime_openlibs_phase2(lua_State *L);

#endif  // INC_RUNTIME_LIBS_H_
