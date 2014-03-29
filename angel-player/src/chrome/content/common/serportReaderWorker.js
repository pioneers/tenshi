/* jshint globalstrict: true */
"use strict";

importScripts("resource://gre/modules/workers/require.js");
let serport = require('chrome://angel-player/content/common/serport_linux');

// TODO(rqou): I think this needs to be done again???
serport.init();
let serportObj = null;

onmessage = function(e) {
    // We assume we only ever get one message, the start message
    serportObj = serport.SerialPortClone(e.data.fd, e.data.timeout);
    while (true) {
        // TODO(rqou): More intelligent reading/timeout-ing
        let buf = serportObj.read(1);
        if (buf.length == 1) {
            postMessage(buf);
        }
    }
};
