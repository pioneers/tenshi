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
