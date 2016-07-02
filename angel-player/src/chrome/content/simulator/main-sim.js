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

const global_state = require('tenshi/common/global_state');
const ubjson = require('tenshi/common/ubjson');

const lua_supp = require('tenshi/lang-support/lua');
const emcc_tools = require('tenshi/common/emcc_tools');
// TODO(rqou): Unify this?
let lua_core;

let tenshiRuntimeInit,
    tenshiRuntimeDeinit,
    loadStudentcode,
    actorSetRunnable,
    tenshiRunQuanta,
    tenshiRegisterCFunctions,
    tenshiFlagSensor;
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
  tenshiFlagSensor = lua_core.cwrap('TenshiFlagSensor', 'number',
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
  // Really hacky, flag all sensors for updates
  for (let name in supported_devices) {
    if (supported_device_info[name].type === 'sensor') {
      let ret = tenshiFlagSensor(runtime, supported_devices[name]);
      if (ret !== 0) {
        console.error("TenshiFlagSensor error!");
      }
    }
  }

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

//////////////////////////////// Kludgey radio ////////////////////////////////

let last_radio_packet;
let last_radio_packet_new;

function attach_new_radio() {
  global_state.get('main_radio').on('send_object', function(obj) {
    let buf = ubjson.encode(obj);
    last_radio_packet = String.fromCharCode.apply(null, buf);
    last_radio_packet_new = true;
  });
}

function get_radio_val() {
  if (last_radio_packet_new) {
    last_radio_packet_new = false;
    return last_radio_packet;
  } else {
    return null;
  }
}

//////////////////// Functions to be called by student code ////////////////////

const runtime_funcs = [
  ['get_device', get_device],
  ['del_device', del_device],
  ['query_dev_info', query_dev_info],
  ['set_simmotor_val', set_simmotor_val],
  ['get_analogsensor_val', get_analogsensor_val],
  ['get_radio_val', get_radio_val],
];

const supported_devices = {
  'sim-motor0': 1,
  'sim-motor1': 2,
  'sim-motor2': 3,
  'sim-motor3': 4,
  'sim-rangefinder': 5,
};

const supported_devices_reverse = {
  1: 'sim-motor0',
  2: 'sim-motor1',
  3: 'sim-motor2',
  4: 'sim-motor3',
  5: 'sim-rangefinder',
};

const supported_device_info = {
  'sim-motor0': {
    'type': 'actuator',
    'dev': 'simmotor',
    '__portidx': 0,
  },
  'sim-motor1': {
    'type': 'actuator',
    'dev': 'simmotor',
    '__portidx': 1,
  },
  'sim-motor2': {
    'type': 'actuator',
    'dev': 'simmotor',
    '__portidx': 2,
  },
  'sim-motor3': {
    'type': 'actuator',
    'dev': 'simmotor',
    '__portidx': 3,
  },
  'sim-rangefinder': {
    'type': 'sensor',
    'dev': 'analogsensor',
    '__portidx': 0,
  },
};

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
  let dev_block = supported_device_info[dev_name];
  if (dev_block) {
    return dev_block[query_type];
  }
}

function set_simmotor_val(obj, val) {
  let idx =
    supported_device_info[supported_devices_reverse[obj.func]].__portidx;
  simmy.robot.setMotor(idx, val);
  // TODO(ericnguyen): increment the version
}

function get_analogsensor_val(obj) {
  let idx =
    supported_device_info[supported_devices_reverse[obj.func]].__portidx;
  return simmy.robot.sensors[idx].getVal();
}

////////////////////////////////// Init stuff //////////////////////////////////

exports.init = function(_window) {
  $(function() {
    simmy = new Simulator(document.getElementById("sim-mainScreen"), null, "testMap");

    attach_new_radio();

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
