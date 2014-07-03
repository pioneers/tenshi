/* jshint globalstrict: true */
"use strict";

const { Editor } = require('tenshi/simulator/Editor');
const { KeyManager, DOWN } = require('tenshi/simulator/KeyManager');
const G = require('tenshi/simulator/window_imports').globals;

exports.init = function(_window) {
  G.window = _window;
  G.document = _window.document;
  G.$ = _window.$;
  G.Ammo = _window.Ammo;
  G.THREE = _window.THREE;

  G.$(function() {
    var eddy = new Editor(G.document, G.document.getElementById("classSelector"),
      G.document.getElementById("objSelector"),
      G.document.getElementById("addingInfo"),
      G.document.getElementById("selnInfo"),
      G.document.getElementById("mainScreen"), null, "blankMap");
    var curSelection = null;
    var keyMan = new KeyManager(G.document);

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

    G.$('#Select').click(function() { eddy.toggleSelection(); });
    G.$('#Move').click(function() { eddy.setMode("MOVE"); });
    G.$('#Resize').click(function() { eddy.setMode("RESIZE"); });
    G.$('#Rotate').click(function() { eddy.setMode("ROTATE"); });
    G.$('#Delete').click(function() { eddy.removeObj(); });

    G.$('#classSelector').change(function() {
      eddy.updateObjSelection();
      eddy.updateAttrSelection();
    });
    G.$('#objSelector').change(function() { eddy.updateAttrSelection(); });
  });
 };
 