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

__device_metatable = {
    __gc = function(obj)
        __runtimeinternal.del_device(obj.__raw)
    end
}

function get_device(id)
    local dev_raw = __runtimeinternal.get_device(id)
    if dev_raw == nil then return nil end

    local dev = {
        __raw = dev_raw,
        -- Cache the type of device
        __dev = __runtimeinternal.query_dev_info(dev_raw, "dev")
    }
    setmetatable(dev, __device_metatable)

    -- Are we a sensor or an actuator?
    local sensor_actuator = __runtimeinternal.query_dev_info(dev_raw, "type")

    -- TODO(rqou): Specialization for some types of sensors/actuators like
    -- Grizzly, etc.

    if sensor_actuator == "sensor" then
        -- Signal objects are extensions of the Lua-side sensor object and
        -- contain the following additional fields:
        -- __downstream: mailbox to forward messages to
        -- value: last value

        -- However, this file does not contain all of the signal/sensor logic;
        -- the updating logic is inside sensor_actor.lua.

        -- Add ourselves to a global map of dev_raw to Lua devices
        local devmap = __runtimeinternal.get_registry()['tenshi.sensorDevMap']
        devmap[dev_raw] = dev

        return dev
    elseif sensor_actuator == "actuator" then
        -- Handle actuators (we return an actuator/mailbox object)
        local actuator = __actuators.create_actuator(dev)

        -- Note that there is also logic in actuator_actor.lua to call the
        -- external-to-runtime update functions

        return actuator
    else
        -- What are we?!
        return nil
    end
end
