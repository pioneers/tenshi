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

/*
All rangefinders are a single box : it shoots a ray out at its <0, 0, 1> direction
    run shoots a ray and sets its value to the distance
*/

const window = require('tenshi/common/window')();
let {Ammo} = window;
const { rotateV3ByQuat, distance } = require('tenshi/simulator/Ammo/mathSupplements');

function Rangefinder(simulator, width, length, height, mass, iniX, iniY, iniZ)
{
    this.simulator = simulator;
    this.physicsObject = simulator.createBox(width, length, height, mass, 0x000000, iniX, iniY, iniZ);
}

Rangefinder.prototype.getVal = function()
{
    return this.value;
};

Rangefinder.prototype.run = function()
{
    var tr = new Ammo.btTransform();
    this.physicsObject.getMotionState().getWorldTransform(tr);

    var pos = tr.getOrigin();
    var rot = tr.getRotation();
    var upQ = new Ammo.btQuaternion(0, 0, 1, 0);

    var nPos = rotateV3ByQuat(upQ, rot);

    // raytracer raytraces start-point to end-point, so 1000 creates a distance raycast of magnitude 1000
    nPos = new Ammo.btVector3(pos.x() + 1000*nPos.x(), pos.y() + 1000*nPos.y(), pos.z() + 1000*nPos.z());

    this.raycast = new Ammo.ClosestRayResultCallback(pos, nPos);

    this.simulator.physicsWorld.rayTest(pos, nPos, this.raycast);
    if(this.raycast.hasHit)
    {
        var end = this.raycast.get_m_hitPointWorld();

        this.value = distance(pos, end);
    }
};

exports.Rangefinder = Rangefinder;
