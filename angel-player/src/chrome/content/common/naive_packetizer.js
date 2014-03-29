/* jshint globalstrict: true */
"use strict";

const { ChromeWorker } = require('chrome');

exports.sendPacketizedData = function(data) {
    // TODO(rqou): Proper lifecycle management for serial port interface
    let serportWorker = new ChromeWorker("common/serportWorker.js");
    // TODO(rqou): Don't hardcode
    serportWorker.postMessage({cmd: "open", data: "/dev/ttyUSB0"});

    serportWorker.onmessage = function(e) {
        let asdfbuf = e.data;
        for (let i = 0; i < asdfbuf.length; i++) {
            dump(asdfbuf[i]);
        }
    };

    // TODO(rqou): Send actual data!
    let buf = new Uint8Array(128);
    for (let i = 0; i < 128; i++) {
        buf[i] = i;
    }
    serportWorker.postMessage({cmd: "write", data: buf});

    // TODO(rqou): Figure out when to close!
};
