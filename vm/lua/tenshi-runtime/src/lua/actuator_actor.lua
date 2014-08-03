-- Licensed to Pioneers in Engineering under one
-- or more contributor license agreements.  See the NOTICE file
-- distributed with this work for additional information
-- regarding copyright ownership.  Pioneers in Engineering licenses
-- this file to you under the Apache License, Version 2.0 (the
-- "License"); you may not use this file except in compliance
--  with the License.  You may obtain a copy of the License at
--
--    http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing,
-- software distributed under the License is distributed on an
-- "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
-- KIND, either express or implied.  See the License for the
-- specific language governing permissions and limitations
-- under the License

-- This file will be run at the end of every time quanta. It is expected to run
-- and then exit. It converts messages sent to mailboxes into functions that
-- update actuators.

for a,_ in
    pairs(__runtimeinternal.get_registry()['tenshi.changed_actuators']) do
    -- a is an actual raw actuator object
    print("a is " .. tostring(a))
    print("a.__lua_dev is " .. tostring(a.__lua_dev))
    print("type is " .. a.__lua_dev.__dev)
    local set_func = __runtimeinternal['set_' .. a.__lua_dev.__dev .. '_val']
    print("set_func is " .. tostring(set_func))

    local val = a:recv({timeout=0})
    print("val is " .. tostring(val))
    while val ~= nil do
        set_func(a.__lua_dev.__raw, val)
        val = a:recv({timeout=0})
        print("val is " .. tostring(val))
    end
end
