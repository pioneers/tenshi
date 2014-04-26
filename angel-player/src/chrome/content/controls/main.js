/* jshint globalstrict: true */
"use strict";

let window,
    $;

let naive_packetizer = require('tenshi/common/naive_packetizer'),
    texteditor = require('tenshi/texteditor/editor'),
    console = require('tenshi/console/console'),
    pieles = require('tenshi/pieles/pieles');

let current_vm = null,
    vm_generator = function () {return null;};

exports.set_vm_generator = function (x) {
    vm_generator = x;
};

exports.init = function(_window) {
    window = _window;
    $ = window.$;

    $("#toggle").click(toggle_running);
    $("#compile").click(compile_code);

    pieles.attachToPage(window);
};

function run() {
    if (current_vm !== null) {
        return; // Do not run multiple simultaneous vms
    }

    if (!texteditor.initialized()) {
        $("#status").text("ERROR: text editor not yet loaded");
        throw "ERROR: text editor not yet loaded";
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
        if (!console.initialized()) {
            return;
        }
        console.report_error(err);
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

function compile_code() {
    if (!texteditor.initialized()) {
        $("#status").text("ERROR: text editor not yet loaded");
        throw "ERROR: text editor not yet loaded";
    }

    var text = texteditor.get_text();
    var compiler_vm = vm_generator();

    try {
        compiler_vm.load_text(text);
        var pkg = compiler_vm.get_pkg('ARM');
        naive_packetizer.sendPacketizedData(pkg);
    } catch(err) {
        if (!console.initialized()) {
            return;
        }
        console.report_error(err);
        throw err;
    }
}
