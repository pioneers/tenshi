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

local triggers = {}

-- This function returns a mailbox that gets an item every time the signal
-- gets a sample from the hardware/external code.
function triggers.sampled(signal)
    local mbox = __mboxinternal.create_independent_mbox()
    signal.__downstream = mbox

    return mbox
end

-- This function returns a mailbox that gets an item every time the signal
-- gets a sample that is different from the previous data.
function triggers.changed(signal)
    local mbox = __mboxinternal.create_independent_mbox()
    signal.__downstream = mbox

    local changed_actor = start_actor(function()
        local last_val
        while true do
            local val = mbox:recv()
            if val ~= last_val then
                get_own_actor():send({val})
            end
            last_val = val
        end
    end)

    return changed_actor
end

return triggers
