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

local piemos = {}

local channels = {
    digital = {},
    analog  = {}
}

-- Initialize all the channels.
for i = 1, 7 do
    channels.analog[i] = { value = 0.0 }
end

for i = 1, 8 do
    channels.digital[i] = { value = false }
end


function piemos.get_axis(idx)
    return channels.analog[idx].value
end

function piemos.get_button(idx)
    return channels.digital[idx].value
end

function piemos.get_channel(name)
    -- Name should be of the form PiEMOSAnalogVals-1 throught
    -- PiEMOSAnalogVals-7 or PiEMOSDigitalVals-1 through PiEMOSDigitalVals-8.
    local dashidx = name:find("-")
    local array = name:sub(1, dashidx - 1)
    local idx = tonumber(name:sub(dashidx + 1))

    if array == 'PiEMOSAnalogVals' then
        return channels.analog[idx]
    elseif array == 'PiEMOSDigitalVals' then
        return channels.digital[idx]
    end
end

function piemos.__process_radio()

    for i = 1, 7 do
        local s = channels.analog[i]
        s.value = __runtimeinternal.get_piemos_analog_val(i)
        -- If there is a __downstream mailbox, send data to it
        if s.__downstream then
            s.__downstream:send({val})
        end
    end

    for i = 1, 8 do
        local s = channels.digital[i]
        s.value = __runtimeinternal.get_piemos_digital_val(i)
        -- If there is a __downstream mailbox, send data to it
        if s.__downstream then
            s.__downstream:send({val})
        end
    end
end

return piemos
