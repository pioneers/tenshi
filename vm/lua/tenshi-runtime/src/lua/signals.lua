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

-- Signal objects are supposed to be an abstract representation of a
-- "real-life" continuous-time signal. However, when data reaches our
-- controller, it is already sampled. So all we do is wrap the mailbox object
-- corresponding to an input from sensors so that it cannot be directly
-- accessed except via the triggers module. We also implement a "value" field
-- so that studentcode can bypass the messaging API if it wishes. This module
-- is not directly exposed to students - it is loaded as "__signals".

local signals = {}

return signals
