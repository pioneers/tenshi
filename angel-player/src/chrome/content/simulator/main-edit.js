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

const { Editor } = require('tenshi/simulator/Editor');
const { KeyManager, DOWN } = require('tenshi/simulator/KeyManager');
const window = require('tenshi/common/window')();
let {document, $, Ammo, THREE} = window;

exports.init = function() {
  $(function() {
    var eddy = new Editor(document, document.getElementById("classSelector"),
      document.getElementById("objSelector"),
      document.getElementById("addingInfo"),
      document.getElementById("selnInfo"),
      document.getElementById("eddy-mainScreen"), null, "blankMap");
    var curSelection = null;
    var keyMan = new KeyManager(document);

    // Bind camera controls to WASD, shift, space (minecraft style)
    keyMan.bindFunc(DOWN, 68, function() {eddy.simulator.cameraController.strafe(3);});
    keyMan.bindFunc(DOWN, 65, function() {eddy.simulator.cameraController.strafe(-3);});

    keyMan.bindFunc(DOWN, 87, function() {eddy.simulator.cameraController.forward(3);});
    keyMan.bindFunc(DOWN, 83, function() {eddy.simulator.cameraController.forward(-3);});

    keyMan.bindFunc(DOWN, 32, function() {eddy.simulator.cameraController.elevate(3);});
    keyMan.bindFunc(DOWN, 16, function() {eddy.simulator.cameraController.elevate(-3);});

    // Bind object manipulation to IJKLUO keys
    keyMan.bindFunc(DOWN, 73, function() { eddy.manipObj(0); });
    keyMan.bindFunc(DOWN, 75, function() { eddy.manipObj(1); });

    keyMan.bindFunc(DOWN, 74, function() { eddy.manipObj(2); });
    keyMan.bindFunc(DOWN, 76, function() { eddy.manipObj(3); });

    keyMan.bindFunc(DOWN, 85, function() { eddy.manipObj(4); });
    keyMan.bindFunc(DOWN, 79, function() { eddy.manipObj(5); });

    eddy.updateObjSelection();
    eddy.updateAttrSelection();
    eddy.simulator.render();

    $('#Select').click(function() { eddy.toggleSelection(); });
    $('#Move').click(function() { eddy.setMode("MOVE"); });
    $('#Resize').click(function() { eddy.setMode("RESIZE"); });
    $('#Rotate').click(function() { eddy.setMode("ROTATE"); });
    $('#Delete').click(function() { eddy.removeObj(); });

    $('#classSelector').change(function() {
      eddy.updateObjSelection();
      eddy.updateAttrSelection();
    });
    $('#objSelector').change(function() { eddy.updateAttrSelection(); });
  });
 };
 