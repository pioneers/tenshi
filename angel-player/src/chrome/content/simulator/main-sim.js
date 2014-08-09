/* jshint globalstrict: true */
"use strict";

const { Simulator } = require('tenshi/simulator/Simulator');
const { KeyManager, DOWN } = require('tenshi/simulator/KeyManager');
const { saveFrame, loadFrame } = require('tenshi/simulator/miscFuncs');
const window = require('tenshi/common/window')();
let {$, Ammo, document} = window;

const lua_supp = require('tenshi/lang-support/lua');
const emcc_tools = require('tenshi/common/emcc_tools');
// TODO(rqou): Unify this?
let lua_core;

let tenshiRuntimeInit,
    tenshiRuntimeDeinit,
    loadStudentcode,
    actorSetRunnable,
    tenshiRunQuanta,
    tenshiRegisterCFunctions;
try {
  lua_core = require('tenshi/vendor-js/lua');

  tenshiRuntimeInit = lua_core.cwrap('TenshiRuntimeInit', 'number');
  tenshiRuntimeDeinit = lua_core.cwrap('TenshiRuntimeDeinit', null,
    ['number']);
  loadStudentcode = lua_core.cwrap('LoadStudentcode', 'number',
    ['number', 'number', 'number', 'number']);
  actorSetRunnable = lua_core.cwrap('ActorSetRunnable', 'number',
    ['number', 'number']);
  tenshiRunQuanta = lua_core.cwrap('TenshiRunQuanta', 'number',
    ['number']);
  tenshiRegisterCFunctions = lua_core.cwrap('TenshiRegisterCFunctions', null,
    ['number', 'number']);
} catch(e) {
  // OK, running in dev without build.sh
}

let simmy;
let runtime;
// Things like function pointers have to stick around
let lual_reg_data;

////////////////////////// Bitrotted? Angelic support //////////////////////////

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

///////////////////////// New Lua runtime integration /////////////////////////

function runRuntimeQuanta(timestamp) {
  let ret = tenshiRunQuanta(runtime); 
  if (ret !== 0) {
    console.error("TenshiRunQuanta error!");
  }

  window.requestAnimationFrame(runRuntimeQuanta);
}

function alloc_luaL_Reg(arr) {
  // arr should be an array of arrays, as if it was C. However, we will
  // automatically add the {NULL, NULL} entry
  let lual_reg_addr =
    lua_core._malloc(2 * emcc_tools.PTR_SIZE * (arr.length + 1));

  for (let i = 0; i < arr.length; i++) {
    let name = arr[i][0];
    let func = lua_supp.luaify(arr[i][1]);

    let str_addr = emcc_tools.buffer_to_ptr(lua_core,
      lua_core.intArrayFromString(name));
    let func_addr = lua_core.Runtime.addFunction(func);

    emcc_tools.set_ptr(
      lua_core,
      lual_reg_addr + i * (2 * emcc_tools.PTR_SIZE) + 0 * emcc_tools.PTR_SIZE,
      str_addr);
    emcc_tools.set_ptr(
      lua_core,
      lual_reg_addr + i * (2 * emcc_tools.PTR_SIZE) + 1 * emcc_tools.PTR_SIZE,
      func_addr);
  }

  emcc_tools.set_ptr(
    lua_core,
    lual_reg_addr +
      arr.length * (2 * emcc_tools.PTR_SIZE) + 0 * emcc_tools.PTR_SIZE,
    0);
  emcc_tools.set_ptr(
    lua_core,
    lual_reg_addr +
      arr.length * (2 * emcc_tools.PTR_SIZE) + 1 * emcc_tools.PTR_SIZE,
    0);

  return lual_reg_addr;
}

function free_luaL_Reg(lual_reg_addr) {
  let i = 0;
  while (true) {
    let str_addr = emcc_tools.get_ptr(lua_core,
      lual_reg_addr + i * (2 * emcc_tools.PTR_SIZE) + 0 * emcc_tools.PTR_SIZE);
    let func_addr = emcc_tools.get_ptr(lua_core,
      lual_reg_addr + i * (2 * emcc_tools.PTR_SIZE) + 1 * emcc_tools.PTR_SIZE);

    i = i + 1;

    if (str_addr && func_addr) {
      lua_core._free(str_addr);
      lua_core.Runtime.removeFunction(func_addr);
    } else {
      break;
    }
  }
  lua_core._free(lual_reg_addr);
}

exports.load_and_run = function(blob) {
  if (!tenshiRuntimeInit) {
    console.log("No Emscripten output, not running.");
    return;
  } else {
    if (runtime) {
      tenshiRuntimeDeinit(runtime);
      free_luaL_Reg(lual_reg_data);
    }

    let ret;

    runtime = tenshiRuntimeInit();

    lual_reg_data = alloc_luaL_Reg(runtime_funcs);
    tenshiRegisterCFunctions(runtime, lual_reg_data);

    let mainactor_buf = lua_core._malloc(emcc_tools.PTR_SIZE);
    let code_buf = emcc_tools.buffer_to_ptr(lua_core, blob);

    ret = loadStudentcode(runtime, code_buf, blob.length, mainactor_buf);
    if (ret !== 0) throw new Error("LoadStudentcode failed!");

    let mainactor = emcc_tools.get_ptr(lua_core, mainactor_buf);
    lua_core._free(mainactor_buf);
    lua_core._free(code_buf);

    ret = actorSetRunnable(mainactor, 1);
    if (ret !== 0) throw new Error("ActorSetRunnable failed!");

    window.requestAnimationFrame(runRuntimeQuanta);
  }
};

//////////////////// Functions to be called by student code ////////////////////

const runtime_funcs = [
  ['get_device', get_device],
  ['del_device', del_device],
  ['query_dev_info', query_dev_info],
  ['set_testdeviceclass_val', set_testdeviceclass_val],
];

const supported_devices = {
  'testdevice': 1,
}

const supported_devices_reverse = {
  1: 'testdevice',
}

function get_device(str) {
  console.log('get_device called with ' + str);

  if (supported_devices[str]) {
    return {type: 'lightuserdata', 'func': supported_devices[str]};
  } else {
    return null;
  }
}

function del_device(obj) {
  console.log("del_device called with " + obj.func);
}

function query_dev_info(obj, query_type) {
  console.log("query_dev_info called with " + obj.func + ", " + query_type);
  let dev_name = supported_devices_reverse[obj.func];

  if (dev_name === 'testdevice') {
    if (query_type === 'type') {
      return "actuator";
    } else if (query_type === 'dev') {
      return "testdeviceclass";
    }
  }
}

function set_testdeviceclass_val(obj, val) {
  console.log("set_testdeviceclass_val called with " + obj.func + ", " + val);
}

////////////////////////////////// Init stuff //////////////////////////////////

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
