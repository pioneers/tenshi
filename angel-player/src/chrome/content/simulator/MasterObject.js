// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

function MasterObject()
{
    this.robotMotors = {}; // stores motor vals of all robots
    this.robotSensors = {}; // stores sensor vals of all robots
    this.version = 0;
    this.isPaused = 0;
    this.frame = 0;
}

MasterObject.prototype.saveMotors = function()
{
    var robotMotors = {};
    robotMotors.version = this.version;

    for(var robot in this.robotMotors)
    {
        robotMotors[robot] = {};
        for(var motor in this.robotMotors[robot])
        {
            robotMotors[robot][motor] = this.robotMotors[robot][motor];
        }
    }

    return robotMotors;
};

MasterObject.prototype.loadMotors = function(state, vrsn)
{
    for(var robot in state)
    {
        for(var motor in state[robot])
        {
            this.robotMotors[robot][motor] = state[robot][motor];
        }
    }

    this.version = vrsn;
};

exports.MasterObject = MasterObject;
