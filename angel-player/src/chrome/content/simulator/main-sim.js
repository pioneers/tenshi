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

/* jshint globalstrict: true */
"use strict";

const { Simulator } = require('tenshi/simulator/Simulator');
const { KeyManager, DOWN } = require('tenshi/simulator/KeyManager');
const { saveFrame, loadFrame } = require('tenshi/simulator/miscFuncs');
const window = require('tenshi/common/window')();
let {$, Ammo, document} = window;

let simmy;

// TODO(rqou): This has probably bitrotted
function gen_vm() {
  var angelic = require("tenshi/angelic/robot");
  var vm = angelic.make();

  vm.set_common_defs_path('chrome://angel-player/content/common_defs');

  function set_motor ( port, val ) {
    // TODO(ericnguyen): use correct API here
    simmy.robot.setMotor(port, val);
    // TODO(ericnguyen): increment the version
  }

  function get_sensor ( port ) {
    // TODO(ericnguyen): use correct API here
    return simmy.robot.sensors[port].getVal();
  }

  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', function(args) {console.log(args);} ),
    vm.make_exfn ( 1, 'set_motor', set_motor ),
    vm.make_exfn ( 2, 'get_sensor', get_sensor ),
    ] );

  vm.source = "Simulator";
  return vm;
}

function onResume() {
  // Set simulator as default VM target when it is focused
  var controls = require("tenshi/controls/main");

  controls.set_vm_generator(gen_vm);
}


exports.init = function(_window) {
  $(function() {
    simmy = new Simulator(document.getElementById("sim-mainScreen"), null, "testMap");

    // TODO (ericnguyen): wrap all these test functions and organize them elsewhere
    var keyMan = new KeyManager(document);

    var moveForwardRight = function()
    {
      simmy.robot.setMotor(3, -100);
      simmy.robot.setMotor(1, -100);
    };

    var moveForwardLeft = function()
    {
      simmy.robot.setMotor(2, 100);
      simmy.robot.setMotor(0, 100);
    };

    var moveBackLeft = function()
    {
      simmy.robot.setMotor(2, -100);
      simmy.robot.setMotor(0, -100);
    };

    var moveBackRight = function()
    {
      simmy.robot.setMotor(3, 100);
      simmy.robot.setMotor(1, 100);
    };

    var stopLeft = function()
    {
      simmy.robot.setMotor(2, 0);
      simmy.robot.setMotor(0, 0);
    };

    var stopRight = function()
    {
      simmy.robot.setMotor(3, 0);
      simmy.robot.setMotor(1, 0);
    };

    var jump = function()
    {
      simmy.robot.physicsChassi.setActivationState(1);
      simmy.robot.physicsChassi.applyCentralForce(new Ammo.btVector3(0, 10000000, 0));
      simmy.robot.physicsChassi.applyTorqueImpulse(new Ammo.btVector3(0, 1000, 0));
    };

    var readVal = function()
    {
      var val = simmy.robot.sensors[0].getVal();
      document.getElementById("RF number").innerHTML = val;
      window.requestAnimationFrame(readVal);
    };

    var save = function()
    {
      saveFrame(simmy);
    };

    var load = function()
    {
      loadFrame(simmy);
    };

    keyMan.bindFunc(DOWN, 68, function() {simmy.cameraController.strafe(3);});
    keyMan.bindFunc(DOWN, 65, function() {simmy.cameraController.strafe(-3);});

    keyMan.bindFunc(DOWN, 87, function() {simmy.cameraController.forward(3);});
    keyMan.bindFunc(DOWN, 83, function() {simmy.cameraController.forward(-3);});

    keyMan.bindFunc(DOWN, 32, function() {simmy.cameraController.elevate(3);});
    keyMan.bindFunc(DOWN, 16, function() {simmy.cameraController.elevate(-3);});

    simmy.render();
    readVal();
  });
  
  $(onResume); // also call on load
};
