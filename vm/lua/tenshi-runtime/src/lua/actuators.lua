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

local actuators = {}

-- Actuators wrap a mailbox but add the ability to set value to update the
-- actuator (allowing students to bypass the messaging API).

actuators.metatable = {
    __index = function(t, k)
        -- There is a problem here where we get loaded before mboxlib
        -- and it is hard to refactor it to be otherwise. Therefore we use
        -- this hack solution where as soon as this function gets called, we
        -- replace ourselves with the actual mboxlib metatable.
        local mboxlib_meta =
            __runtimeinternal.get_registry()['tenshi.mboxlib.metatable']
        actuators.metatable.__index = mboxlib_meta
        return t[k]
    end,
    __newindex = function(t, k, v)
        if k == 'value' then
            t:send({v})
        else
            t[k] = v
        end
    end
}

function actuators.create_actuator(lua_dev)
    local ret = __mboxinternal.create_independent_mbox()
    ret.__mbox.actuator = 1
    ret.__lua_dev = lua_dev
    setmetatable(ret, actuators.metatable)
    return ret
end

return actuators
