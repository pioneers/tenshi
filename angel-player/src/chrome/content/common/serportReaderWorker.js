// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

/* jshint globalstrict: true */
"use strict";

importScripts("resource://gre/modules/workers/require.js");
/* global navigator */
let is_win = navigator.platform.search(/win/i) > -1;
let serport = is_win ?
    require('chrome://angel-player/content/common/serport_win') :
    require('chrome://angel-player/content/common/serport_posix');

// TODO(rqou): I think this needs to be done again???
serport.init();
let serportObj = null;

const PACKET_STATE_IDLE = 0;
const PACKET_STATE_LEN = 1;
let current_packet_state = PACKET_STATE_IDLE;

// TODO(rqou): I don't want to import all of Typpo here
const XBEE_MAGIC = 0x7E;

// onmessage is magically called when a message is sent to this worker.
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
            if (buf.length === 0) {
                // Timed out, just try again
                break;
            }
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
            serportObj.timeout = 1;
            let combinedBuf = new Uint8Array(mainData.length + stubBuf.length);
            combinedBuf.set(stubBuf, 0);
            combinedBuf.set(mainData, stubBuf.length);
            postMessage(combinedBuf);

            current_packet_state = PACKET_STATE_IDLE;
        }
    }
};
