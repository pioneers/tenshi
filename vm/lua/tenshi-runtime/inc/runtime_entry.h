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

#ifndef INC_RUNTIME_ENTRY_H_
#define INC_RUNTIME_ENTRY_H_

#include <stdlib.h>
#include "lua.h"      // NOLINT(build/include)
#include "lauxlib.h"  // NOLINT(build/include)

typedef struct _TenshiRuntimeState* TenshiRuntimeState;

#include "inc/actor_sched.h"

#define QUANTA_OPCODES  1000

#define RIDX_RUNTIMESTATE     "tenshi.globalCState"
#define RIDX_CHANGED_SENSORS  "tenshi.changed_sensors"
#define RIDX_SENSORDEVMAP     "tenshi.sensorDevMap"

extern TenshiRuntimeState TenshiRuntimeInit(void);
extern void TenshiRuntimeDeinit(TenshiRuntimeState s);

// Returns the same values as lua_load. Automatically creates a new actor for
// the loaded code.
extern int LoadStudentcode(TenshiRuntimeState s, const char *data, size_t len,
  TenshiActorState *actor_state);

// Runs the interpreter for QUANTA_OPCODES opcodes. Automatically handles
// scheduling, etc. Returns LUA_OK on success.
extern int TenshiRunQuanta(TenshiRuntimeState s);

// Used for e.g. sending data to/from sensors/actuators
extern void TenshiMainStackPushInt(TenshiRuntimeState s, lua_Integer i);
extern void TenshiMainStackPushUInt(TenshiRuntimeState s, lua_Unsigned i);
extern void TenshiMainStackPushFloat(TenshiRuntimeState s, lua_Number i);
// Also pops the item off the stack.
extern lua_Integer TenshiMainStackGetInt(TenshiRuntimeState s);
extern lua_Unsigned TenshiMainStackGetUInt(TenshiRuntimeState s);
extern lua_Number TenshiMainStackGetFloat(TenshiRuntimeState s);

// Registers the functions in l into the __runtimeinternal module.
// The following functions are expected to be registered (this is the
// "contract" between the runtime and the outside world):
//  * get_device(id) --> lightuserdata
//      Called when the student code calls get_device. Should return a
//      lightuserdata containing internal data needed to talk with this device
//      or nil if the external code can guarantee this device doesn't exist.
//  * del_device(lightuserdata)
//      Called when the Lua side of the sensor/actuator device is GC'd so that
//      the external code can free memory.
//  * query_dev_info(lightuserdata, attr) --> obj
//      Gets some information regarding the device. Attr is a string that
//      specifies what is being querried. Some strings that will be used:
//        * "type": Returns either "sensor" or "actuator"
//        * "dev": Returns the type of device, e.g. "grizzly" or "switch"
//  * get_<dev>_val(lightuserata) --> obj
//      Gets the "main" data for a sensor. <dev> is the return value of
//      query_dev_info(lightuserdata, "dev")
//      For example, get_switch_val(lightuserdata) will return a boolean
//      corresponding to if the switch is pressed
//  * set_<dev>_val(lightuserdata, obj) --> nil
//      Sets the "main" data for an actuator. <dev> is the return value of
//      query_dev_info(lightuserdata, "dev")
//      For example, set_grizzly_val(lightuserdata, speed)
//  Other methods may be present depending on the type of device.
extern void TenshiRegisterCFunctions(TenshiRuntimeState s, const luaL_Reg *l);

// Flag the sensor with lightuserdata "dev" as having a new sample. The
// runtime will then call the appropriate get_<dev>_val function. Returns
// LUA_OK on success.
extern int TenshiFlagSensor(TenshiRuntimeState s, const void *const dev);

#endif  // INC_RUNTIME_ENTRY_H_
