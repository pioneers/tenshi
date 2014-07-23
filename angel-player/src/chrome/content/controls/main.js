/* jshint globalstrict: true */
"use strict";

let window = require('tenshi/common/window')();
let {$} = window;

let naive_packetizer = require('tenshi/common/naive_packetizer'),
    texteditor = require('tenshi/texteditor/editor'),
    studentconsole = require('tenshi/console/console'),
    pieles = require('tenshi/pieles/pieles');

let current_vm = null,
    vm_generator = function () {return null;};

exports.set_vm_generator = function (x) {
    vm_generator = x;
};

exports.init = function() {
    $("#toggle").click(toggle_running);
    $("#compile_ngl").click(compile_ngl);
    $("#compile_lua").click(compile_lua);

    pieles.attachToPage(window);
};

function run() {
    if (current_vm !== null) {
        return; // Do not run multiple simultaneous vms
    }

    var text = texteditor.get_text();
    current_vm = vm_generator();

    if (current_vm === null) {
        $("#status").text("ERROR: no targets are open. Try opening Simulator or Console");
        throw "ERROR: no vm could be created. Try opening a valid VM target page";
    }

    try {
        current_vm.load_text(text);
        current_vm.start_main();
    } catch(err) {
        studentconsole.report_error(err);
        throw err;
    }
    current_vm.set_evaluation_rate(100, 0.01);
}

function stop() {
    if (current_vm === null) {
        return; // No vm is running
    }

    // These arguments are special-cased to fully stop the VM
    current_vm.set_evaluation_rate(0, 0);
    current_vm = null;
}

function toggle_running() {
    if (current_vm === null) {
        run();
        $("#toggle").text("STOP");
        $("#status").text("Running in: " + current_vm.source);
    } else {
        stop();
        $("#toggle").text("RUN");
        $("#status").text("");
    }
}

function compile_ngl() {
    var text = texteditor.get_text();
    var compiler_vm = vm_generator();

    compiler_vm.load_text(text);
    var pkg = compiler_vm.get_pkg('ARM');
    naive_packetizer.sendPacketizedData(pkg);
}

function compile_lua() {
    var text = texteditor.get_text();

    let lua = require('tenshi/vendor-js/lua');
    let lua_state = lua.ccall('luaL_newstate', 'number');
    if (lua.ccall('luaL_loadstring', 'number',
        ['number', 'string'], [lua_state, text]) !== 0) {
        // TODO(rqou): Report the actual error
        throw "Didn't load properly!";
    }

    let lua_bytecode = new Uint8Array(0);
    function lua_dump_callback(L, p, sz, ud) {
        let lua_bytecode_new = new Uint8Array(lua_bytecode.length + sz);
        lua_bytecode_new.set(lua_bytecode);
        lua_bytecode_new.set(
            lua.HEAPU8.subarray(p, p + sz), lua_bytecode.length);
        lua_bytecode = lua_bytecode_new;
    }
    let lua_dump_callback_ptr = lua.Runtime.addFunction(lua_dump_callback);
    if (lua.ccall('lua_dump', 'number',
        ['number', 'number', 'number'],
        [lua_state, lua_dump_callback_ptr, 0]) !== 0) {
        throw "Didn't dump properly!";
    }
    lua.ccall('lua_close', null, ['number'], [lua_state]);
    lua.Runtime.removeFunction(lua_dump_callback_ptr);

    naive_packetizer.sendPacketizedData(lua_bytecode);
}
