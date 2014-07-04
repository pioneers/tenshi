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
      document.getElementById("mainScreen"), null, "blankMap");
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
 