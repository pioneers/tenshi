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
All motors consist of a box and a cylinder shape
    getBox gets the motor collidable shape
    getWheel gets the wheel collidable shape
    run reads the value stored, and runs the motor
*/

const window = require('tenshi/common/window')();
let {Ammo} = window;

const { rotateV3ByQuat } = require('tenshi/simulator/Ammo/mathSupplements');

function Motor(simulator, wheelRad, wheelHeight, wheelMass, wheelColor, iniX, iniY, iniZ)
{
    this.wheel = simulator.createCylinder(wheelRad, wheelHeight, wheelMass, wheelColor, iniX, iniY - wheelHeight, iniZ + wheelRad);

    // friction in sideways direction should be high, relative to rolling
    this.wheel.setAnisotropicFriction(new Ammo.btVector3(0.1, 1, 0.1));
    this.value = 0;

    return this;
}

Motor.prototype.getWheel = function()
{
    return this.wheel;
};

Motor.prototype.getVal = function()
{
    return this.value;
};

Motor.prototype.setVal = function(val)
{
    this.value = val;
};

Motor.prototype.run = function()
{
    var value = this.value;
    var tr = new Ammo.btTransform();
    this.wheel.getMotionState().getWorldTransform(tr);

    var pos = tr.getOrigin();
    var rot = tr.getRotation();
    var upQ = new Ammo.btVector3(0, 1, 0);

    var newQ = rotateV3ByQuat(upQ, rot);
        newQ = new Ammo.btVector3(value*newQ.x(), value*newQ.y(), value*newQ.z());

    // torque only acts on activated objects
    this.wheel.setActivationState(1);
    this.wheel.applyTorqueImpulse(newQ);
};

exports.Motor = Motor;
