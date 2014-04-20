/* jshint globalstrict: true */
"use strict";

importScripts("resource://gre/modules/workers/require.js");
let serport = require('chrome://angel-player/content/common/serport_linux');

serport.init();
let port = null;
let serportObj = null;
let readerWorker = null;

function readerWorkerPassthrough(e) {
    postMessage(e.data);
}

// onmessage is magically called when a message is sent to this worker.
onmessage = function(e) {
    if (e.data.cmd === "open") {
        port = e.data.data;
        // We start with a (long) 1 sec timeout; The reader might adjust the
        // timeout as appropriate while receiving packet parts. NOTE that
        // passing a null timeout (infinite) causes XULrunner to get kinda
        // confused and you won't be able to properly terminate the thread.
        // TODO(rqou): Don't hardcode baud
        serportObj = serport.SerialPortOpen(port, 57600, 1);
        readerWorker = new ChromeWorker('serportReaderWorker.js');
        readerWorker.onmessage = readerWorkerPassthrough;
        readerWorker.postMessage(
            {timeout: serportObj.timeout,
             fd: serportObj.fd});
    } else if (e.data.cmd === "write") {
        serportObj.write(e.data.data);
    } else if (e.data.cmd === "close") {
        readerWorker.terminate();
        serportObj.close();
        close();
    }
};
