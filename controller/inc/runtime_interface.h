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

#ifndef INC_RUNTIME_INTERFACE_H_
#define INC_RUNTIME_INTERFACE_H_

#include <lua.h>
#include <lualib.h>

// Interpreter runtime
#include <inc/mboxlib.h>
#include <inc/runtime_entry.h>

#include "inc/runtime.h"


void runtime_register(TenshiRuntimeState s);
void lua_register_all(lua_State *L);

int lua_time(lua_State *L);

int lua_get_device(lua_State *L);
int lua_del_device(lua_State *L);
int lua_query_dev_info(lua_State *L);


int lua_set_radio_val(lua_State *L);  // Special psudo-smart sensor for radio
int lua_get_radio_val(lua_State *L);

int lua_set_status_led_val(lua_State *L);
int lua_get_button_val(lua_State *L);
int lua_get_switch_val(lua_State *L);
int lua_set_led_val(lua_State *L);
int lua_get_analog_val(lua_State *L);
int lua_set_analog_val(lua_State *L);
int lua_set_grizzly_val(lua_State *L);



void setAllSmartSensorGameMode(RuntimeMode mode);


#endif  // INC_RUNTIME_INTERFACE_H_
