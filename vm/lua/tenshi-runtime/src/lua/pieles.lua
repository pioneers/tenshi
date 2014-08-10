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

local pieles = {}

local watched_channels = {}
setmetatable(watched_channels, {__mode = 'k'})

function pieles.get_channel(name)
    -- This is currently really hardcoded. The name passed in here is of the
    -- form <arrname>-<idx>, where arrname can be either PiELESAnalogVals or
    -- PiELESDigitalVals and idx can be from 0-7 for digital and 0-6 for analog

    local dashidx = name:find("-")

    ret = {
        __arr = name:sub(1, dashidx - 1),
        __idx = tonumber(name:sub(dashidx + 1))
    }

    watched_channels[ret] = 1

    return ret
end

function pieles.__process_radio()
    local packet = __runtimeinternal.get_radio_val()
    if packet ~= nil then
        local obj = ubjson.decode(packet, 1)

        for s,_ in pairs(watched_channels) do
            local val = obj[s.__arr][s.__idx]

            -- Update value
            s.value = val
            -- If there is a __downstream mailbox, send data to it
            if s.__downstream then
                s.__downstream:send({val})
            end
        end
    end
end

return pieles
