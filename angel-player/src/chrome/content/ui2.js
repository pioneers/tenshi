/* jshint globalstrict: true */
"use strict";

var serport = require('tenshi/common/serport_linux');

exports.init = function(_window) {
    serport.init();
    var port = serport.SerialPortOpen("/dev/ttyUSB0", 115200);
    var buf = new Uint8Array(256);
    for (let i = 0; i < 256; i++) {
        buf[i] = i;
    }
    port.write(buf);
    var buf2 = port.read(256);
    for (let i = 0; i < 256; i++) {
        dump(buf2[i]);
        dump('\n');
    }
    port.close();
};
