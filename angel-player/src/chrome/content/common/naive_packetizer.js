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
            dump("\n");
        }

        dump("\n\n\n");
    };

    // TODO(rqou): Send actual data!
    let buf = new Uint8Array(6);
    buf[0] = 0x7E;
    buf[1] = 0x00;
    buf[2] = 0x02;
    buf[3] = 0xAA;
    buf[4] = 0x55;
    buf[5] = 0x33;
    serportWorker.postMessage({cmd: "write", data: buf});

    // TODO(rqou): Figure out when to close!
};
