/* jshint globalstrict: true */
"use strict";

// This is the toplevel serial port module that a) spawns the serial port read
// thread and b) wraps the serial port code for different platforms.

const { ChromeWorker } = require('chrome');
// TODO(rqou): Non-Linux support
const serport = require('tenshi/common/serport_posix');

serport.init();

exports.open = function(port) {
    // We start with a (long) 1 sec timeout; The reader might adjust the
    // timeout as appropriate while receiving packet parts. NOTE that
    // passing a null timeout (infinite) causes XULrunner to get kinda
    // confused and you won't be able to properly terminate the thread.
    // TODO(rqou): Don't hardcode baud
    let serportObj = serport.SerialPortOpen(port, 57600, 1);
    let readerWorker = new ChromeWorker('common/serportReaderWorker.js');
    readerWorker.postMessage(
        {timeout: serportObj.timeout,
         fd: serportObj.fd});

    let ret = {};
    ret.serportObj = serportObj;
    ret.readerWorker = readerWorker;
    ret.write = writeFunc;
    ret.close = closeFunc;
    ret.setReadHandler = setReadHandlerFunc;

    return ret;
};

function writeFunc(data) {
    /* jshint validthis: true */
    return this.serportObj.write(data);
}

function closeFunc(data) {
    /* jshint validthis: true */
    this.readerWorker.terminate();
    this.serportObj.close();
}

function setReadHandlerFunc(handler) {
    /* jshint validthis: true */

    // TODO(rqou): Only one handler can be set this way. Is that a problem?
    this.readerWorker.onmessage = handler;
}
