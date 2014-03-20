/* jshint globalstrict: true */
"use strict";

var serport = require('tenshi/common/serport_linux');

exports.init = function(_window) {
    serport.init();
    var port = serport.SerialPortOpen("/dev/ttyUSB0", 115200, 0.001);
    var buf = new Uint8Array(128);
    for (let i = 0; i < 128; i++) {
        buf[i] = i;
    }
    port.write(buf);
    var buf2 = port.read(256);
    for (let i = 0; i < buf2.length; i++) {
        dump(buf2[i]);
        dump('\n');
    }
    port.close();
};
