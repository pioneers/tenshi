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

-- This file will be run at the beginning of every time quanta. It is expected
-- to run and then exit. It converts updated sensors into messages sent into
-- mailboxes.

local changed_sensors =
    __runtimeinternal.get_registry()['tenshi.changed_sensors']

for s,_ in pairs(changed_sensors) do
    -- s is a dev_raw lightuserdata
    local dev = __runtimeinternal.get_registry()['tenshi.sensorDevMap'][s]
    -- dev is the sensor object

    if dev ~= nil then
        local val = __runtimeinternal['get_' .. dev.__dev .. '_val'](s)

        -- Update value
        dev.value = val
        -- If there is a __downstream mailbox, send data to it
        if dev.__downstream then
            dev.__downstream:send({val})
        end
    end

    changed_sensors[s] = nil
end

piemos.__process_radio()
pieles.__process_radio()
