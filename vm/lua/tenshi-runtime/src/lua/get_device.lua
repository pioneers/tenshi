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

function get_device(id)
    local dev = __mboxinternal.find_sensor_actuator(id)
    if dev == nil then return nil end

    -- Sensor or actuator?
    -- TODO(rqou): We need to expand this a lot for implementing behavior
    -- beyond the core API

    if dev.__mbox.actuator ~= nil then
        -- It is an actuator
        return __actuators.wrap_actuator(dev)
    else
        -- It is a sensor
        return __signals.wrap_sensor(dev)
    end
end
