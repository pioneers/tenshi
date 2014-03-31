/* jshint globalstrict: true */
"use strict";

let naive_packetizer = require('tenshi/common/naive_packetizer');

let window = null;
let document = null;

exports.init = function(_window) {
    window = _window;
    document = window.document;
    // TODO(rqou): This is a dumb way to expose this function
    window.sendPacketizedData = naive_packetizer.sendPacketizedData;
};

let vm_generator = function () {return null;};

exports.vm_generator = function () {
    return vm_generator();
};

exports.set_vm_generator = function (x) {
    vm_generator = x;
};
