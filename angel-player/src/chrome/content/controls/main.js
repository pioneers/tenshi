/* jshint globalstrict: true */
"use strict";

let window = require('tenshi/common/window')();
let {$} = window;

let naive_packetizer = require('tenshi/common/naive_packetizer'),
    texteditor = require('tenshi/texteditor/editor'),
    studentconsole = require('tenshi/console/console'),
    pieles = require('tenshi/pieles/pieles'),
    lua = require('tenshi/lang-support/lua');

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

    let ret = lua.compile_lua(text);
    if (!ret.success) {
        studentconsole.report_error(ret.error);
        return;
    }

    studentconsole.report_message("Compiled successfully!");
    for (let i = 0; i < ret.bytecode.length; i++) {
        console.log(ret.bytecode[i]);
    }
    naive_packetizer.sendPacketizedData(ret.bytecode);
}
