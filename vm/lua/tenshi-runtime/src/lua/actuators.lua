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
        return t.__actuator[k]
    end,
    __newindex = function(t, k, v)
        if k == 'value' then
            t.__actuator:send({v})
        else
            t[k] = v
        end
    end
}

function actuators.wrap_actuator(actuator)
    local ret = {}
    ret.__actuator = actuator
    setmetatable(ret, actuators.metatable)
    return ret
end

return actuators
