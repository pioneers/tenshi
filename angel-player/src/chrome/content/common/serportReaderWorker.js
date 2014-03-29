/* jshint globalstrict: true */
"use strict";

importScripts("resource://gre/modules/workers/require.js");
let serport = require('chrome://angel-player/content/common/serport_linux');

// TODO(rqou): I think this needs to be done again???
serport.init();
let serportObj = null;

const PACKET_STATE_IDLE = 0;
const PACKET_STATE_LEN = 1;
let current_packet_state = PACKET_STATE_IDLE;

// TODO(rqou): I don't want to import all of Typpo here
const XBEE_MAGIC = 0x7E;

onmessage = function(e) {
    // We assume we only ever get one message, the start message
    serportObj = serport.SerialPortClone(e.data.fd, e.data.timeout);
    // TODO(rqou): Hardcoded for XBee packet structure; ugly.
    let stubBuf = new Uint8Array(3);
    let buf;
    while (true) {
        switch(current_packet_state) {
        case PACKET_STATE_IDLE:
            buf = serportObj.read(1);
            if (buf[0] == XBEE_MAGIC) {
                stubBuf[0] = buf[0];
                current_packet_state = PACKET_STATE_LEN;
            }
            break;
        case PACKET_STATE_LEN:
            buf = serportObj.read(2);
            let length = buf[0] << 8 | buf[1];
            stubBuf[1] = buf[0];
            stubBuf[2] = buf[1];

            // TODO(rqou): Don't hardcode timeout?
            serportObj.timeout = 0.1;
            // Length does not include checksum -- add 1 byte
            let mainData = serportObj.read(length + 1);
            serportObj.timeout = null;
            let combinedBuf = new Uint8Array(mainData.length + stubBuf.length);
            combinedBuf.set(stubBuf, 0);
            combinedBuf.set(mainData, stubBuf.length);
            postMessage(combinedBuf);
        }
    }
};
