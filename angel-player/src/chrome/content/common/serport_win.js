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

/* jshint newcap: false */
// This breaks really hard here because all Win32 API functions contain capital
// letters at the beginning.

// Handles serial ports on Win32 only. Very inspired by pySerial.
// TODO(rqou): Win64 might not work, do we care?

// Load ctypes if we're not in a ChromeWorker
if (typeof ctypes === 'undefined') {
    const { Cu } = require('chrome');
    Cu.import('resource://gre/modules/ctypes.jsm');
}

let _doneBasicInit = false;

let kernel32;
let CreateFile;
let CreateEvent;
let SetupComm;
let GetCommTimeouts;
let SetCommTimeouts;
let SetCommMask;
let GetCommState;
let SetCommState;
let CloseHandle;
let PurgeComm;
let WriteFile;
let ReadFile;
let GetOverlappedResult;
let ResetEvent;
let ClearCommError;
let WaitForSingleObject;

const KERNEL32_PATH = "kernel32.dll";

const INVALID_HANDLE_VALUE = ctypes.voidptr_t(-1);

const GENERIC_READ = 0x80000000;
const GENERIC_WRITE = 0x40000000;
const OPEN_EXISTING = 3;
const FILE_ATTRIBUTE_NORMAL = 0x80;
const FILE_FLAG_OVERLAPPED = 0x40000000;

const struct_OVERLAPPED = new ctypes.StructType(
    'OVERLAPPED',
    [
        { 'Internal': ctypes.uintptr_t },
        { 'InternalHigh': ctypes.uintptr_t },
        // Note: The union with Pointer is unsupported. However, we don't use
        // it anyways.
        { 'Offset': ctypes.uint32_t },
        { 'OffsetHigh': ctypes.uint32_t },
        { 'hEvent': ctypes.voidptr_t },
    ]
    );

const struct_COMMTIMEOUTS = new ctypes.StructType(
    'COMMTIMEOUTS',
    [
        { 'ReadIntervalTimeout': ctypes.uint32_t },
        { 'ReadTotalTimeoutMultiplier': ctypes.uint32_t },
        { 'ReadTotalTimeoutConstant': ctypes.uint32_t },
        { 'WriteTotalTimeoutMultiplier': ctypes.uint32_t },
        { 'WriteTotalTimeoutConstant': ctypes.uint32_t },
    ]
    );

const EV_ERR = 0x0080;

const struct_DCB = new ctypes.StructType(
    'DCB',
    [
        { 'DCBlength': ctypes.uint32_t },
        { 'BaudRate': ctypes.uint32_t },
        { 'f': ctypes.uint32_t },   // Bitfields are not supported in js-ctypes
        { 'wReserved': ctypes.uint16_t },
        { 'XonLim': ctypes.uint16_t },
        { 'XoffLim': ctypes.uint16_t },
        { 'ByteSize': ctypes.uint8_t },
        { 'Parity': ctypes.uint8_t },
        { 'StopBits': ctypes.uint8_t },
        { 'XonChar': ctypes.char },
        { 'XoffChar': ctypes.char },
        { 'ErrorChar': ctypes.char },
        { 'EofChar': ctypes.char },
        { 'EvtChar': ctypes.char },
        { 'wReserved1': ctypes.uint16_t },
    ]
    );

const NOPARITY = 0;
const ODDPARITY = 1;
const EVENPARITY = 2;
const MARKPARITY = 3;
const SPACEPARITY = 4;

const ONESTOPBIT = 0;
const ONE5STOPBITS = 1;
const TWOSTOPBITS = 2;

const fBinary = 0x00000001;
const fParity = 0x00000002;
const fOutxCtsFlow = 0x00000004;
const fOutxDsrFlow = 0x00000008;

const DTR_CONTROL_DISABLE = 0 << 4;
const DTR_CONTROL_ENABLE = 1 << 4;
const DTR_CONTROL_HANDSHAKE = 2 << 4;
const fDtrControl = 0x00000030;

const fDsrSensitivity = 0x00000040;
const fTXContinueOnXoff = 0x00000080;
const fOutX = 0x00000100;
const fInX = 0x00000200;
const fErrorChar = 0x00000400;
const fNull = 0x00000800;

const RTS_CONTROL_DISABLE = 0 << 12;
const RTS_CONTROL_ENABLE = 1 << 12;
const RTS_CONTROL_HANDSHAKE = 2 << 12;
const RTS_CONTROL_TOGGLE = 3 << 12;
const fRtsControl = 0x00003000;

const fAbortOnError = 0x00004000;

const XON = 17;
const XOFF = 19;

const PURGE_RXABORT = 0x0002;
const PURGE_RXCLEAR = 0x0008;
const PURGE_TXABORT = 0x0001;
const PURGE_TXCLEAR = 0x0004;

const ERROR_IO_PENDING = 997;

const struct_COMSTAT = new ctypes.StructType(
    'COMSTAT',
    [
        { 'f': ctypes.uint32_t },
        { 'cbInQue': ctypes.uint32_t },
        { 'cbOutQue': ctypes.uint32_t },
    ]
    );

const INFINITE = 0xFFFFFFFF;

exports.init = function() {
    if (_doneBasicInit) {
        return;
    }

    kernel32 = ctypes.open(KERNEL32_PATH);

    CreateFile = kernel32.declare(
        'CreateFileW',
        ctypes.winapi_abi,
        ctypes.voidptr_t,

        ctypes.jschar.ptr,
        ctypes.uint32_t,
        ctypes.uint32_t,
        ctypes.voidptr_t,   // LPSECURITY_ATTRIBUTES not used
        ctypes.uint32_t,
        ctypes.uint32_t,
        ctypes.voidptr_t
        );

    CreateEvent = kernel32.declare(
        'CreateEventW',
        ctypes.winapi_abi,
        ctypes.voidptr_t,

        ctypes.voidptr_t,   // LPSECURITY_ATTRIBUTES not used
        ctypes.int,         // Note, actually a BOOL (which is int)
        ctypes.int,         // Note, actually a BOOL (which is int)
        ctypes.jschar.ptr
        );

    SetupComm = kernel32.declare(
        'SetupComm',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        ctypes.uint32_t,
        ctypes.uint32_t
        );

    GetCommTimeouts = kernel32.declare(
        'GetCommTimeouts',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        struct_COMMTIMEOUTS.ptr
        );

    SetCommTimeouts = kernel32.declare(
        'SetCommTimeouts',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        struct_COMMTIMEOUTS.ptr
        );

    SetCommMask = kernel32.declare(
        'SetCommMask',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        ctypes.uint32_t
        );

    GetCommState = kernel32.declare(
        'GetCommState',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        struct_DCB.ptr
        );

    SetCommState = kernel32.declare(
        'SetCommState',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        struct_DCB.ptr
        );

    CloseHandle = kernel32.declare(
        'CloseHandle',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t
        );

    PurgeComm = kernel32.declare(
        'PurgeComm',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        ctypes.uint32_t
        );

    WriteFile = kernel32.declare(
        'WriteFile',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        ctypes.uint8_t.ptr,
        ctypes.uint32_t,
        ctypes.uint32_t.ptr,
        struct_OVERLAPPED.ptr
        );

    ReadFile = kernel32.declare(
        'ReadFile',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        ctypes.uint8_t.ptr,
        ctypes.uint32_t,
        ctypes.uint32_t.ptr,
        struct_OVERLAPPED.ptr
        );

    GetOverlappedResult = kernel32.declare(
        'GetOverlappedResult',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        struct_OVERLAPPED.ptr,
        ctypes.uint32_t.ptr,
        ctypes.int
        );

    ResetEvent = kernel32.declare(
        'ResetEvent',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t
        );

    ClearCommError = kernel32.declare(
        'ClearCommError',
        ctypes.winapi_abi,
        ctypes.int,

        ctypes.voidptr_t,
        ctypes.uint32_t.ptr,
        struct_COMSTAT.ptr
        );

    WaitForSingleObject = kernel32.declare(
        'WaitForSingleObject',
        ctypes.winapi_abi,
        ctypes.uint32_t,

        ctypes.voidptr_t,
        ctypes.uint32_t
        );

    _doneBasicInit = true;
};

function serClose() {
    /* jshint validthis: true */

    SetCommTimeouts(this.hComPort, this._orgTimeouts.address());
    CloseHandle(this.hComPort);
    CloseHandle(this._overlappedRead.hEvent);
    CloseHandle(this._overlappedWrite.hEvent);

    // TODO(rqou): This doesn't check for errors. What are you supposed to even
    // do?
}

function serWrite(data) {
    /* jshint validthis: true */

    let d = new ctypes.uint8_t.ptr(data.buffer);

    let n = ctypes.uint32_t();
    let err = +WriteFile(this.hComPort, d, data.length, n.address(),
        this._overlappedWrite.address());
    if (!err && ctypes.winLastError != ERROR_IO_PENDING) {
        throw ("WriteFile failed (" + ctypes.winLastError + ")");
    }
    // if blocking (None) or w/ write timeout (>0)
    // TODO(rqou): Only blocking write supported
    // Wait for the write to complete.
    err = +GetOverlappedResult(
        this.hComPort, this._overlappedWrite.address(), n.address(), true);
    if (+(n.value) !== data.length) {
        throw "writeTimeoutError";
    }
    return +(n.value);
}

function serRead(size) {
    /* jshint validthis: true */

    if (typeof(size) === 'undefined') size = 1;

    if (size > 0) {
        ResetEvent(this._overlappedRead.hEvent);
        let flags = ctypes.uint32_t();
        let comstat = new struct_COMSTAT();
        if (+ClearCommError(
            this.hComPort, flags.address(), comstat.address()) === 0) {
            throw 'call to ClearCommError failed';
        }
        if (this.timeout === 0) {
            let n = Math.min(comstat.cbInQue, size);
            if (n > 0) {
                let readbuf = new Uint8Array(n);
                let buf = new ctypes.uint8_t.ptr(readbuf.buffer);
                let rc = ctypes.uint32_t();
                let err = +ReadFile(this.hComPort, buf, n, rc.address(),
                    this._overlappedRead.address());
                if (!err && ctypes.winLastError != ERROR_IO_PENDING) {
                    throw ("ReadFile failed (" + ctypes.winLastError + ")");
                }
                err = +WaitForSingleObject(this._overlappedRead.hEvent,
                    INFINITE);
                let read = readbuf.subarray(0, +(rc.value));
                return read;
            } else {
                return new Uint8Array(0);
            }
        } else {
            let readbuf = new Uint8Array(size);
            let buf = new ctypes.uint8_t.ptr(readbuf.buffer);
            let rc = ctypes.uint32_t();
            let err = +ReadFile(this.hComPort, buf, size, rc.address(),
                this._overlappedRead.address());
            if (!err && ctypes.winLastError != ERROR_IO_PENDING) {
                throw ("ReadFile failed (" + ctypes.winLastError + ")");
            }
            err = +GetOverlappedResult(this.hComPort,
                this._overlappedRead.address(), rc.address(), true);
            let read = readbuf.subarray(0, +(rc.value));
            return read;
        }
    } else {
        return new Uint8Array(0);
    }
}

exports.SerialPortOpen = function(port, baud, timeout) {

    if (typeof(timeout) === 'undefined') timeout = null;

    // TODO(rqou): Proper behavior for \\.\?
    let hComPort = CreateFile(port,
        GENERIC_READ + GENERIC_WRITE,
        0, // exclusive access
        null, // no security
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        null);
    if (hComPort.toString() === INVALID_HANDLE_VALUE.toString()) {
        throw ("could not open port " + port + ": " + ctypes.winLastError);
    }

    let _overlappedRead = new struct_OVERLAPPED();
    _overlappedRead.hEvent = CreateEvent(null, 1, 0, null);
    if (+_overlappedRead.hEvent === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        throw ("CreateEvent failed: " + ctypes.winLastError);
    }
    let _overlappedWrite = new struct_OVERLAPPED();
    _overlappedWrite.hEvent = CreateEvent(null, 0, 0, null);
    if (+_overlappedWrite.hEvent === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        CloseHandle(_overlappedRead.hEvent);
        throw ("CreateEvent failed: " + ctypes.winLastError);
    }

    // Setup a 4k buffer
    if (+SetupComm(hComPort, 4096, 4096) === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        CloseHandle(_overlappedRead.hEvent);
        CloseHandle(_overlappedWrite.hEvent);
        throw ("SetupComm failed: " + ctypes.winLastError);
    }

    // Save original timeout values:
    let _orgTimeouts = new struct_COMMTIMEOUTS();
    if (+GetCommTimeouts(hComPort, _orgTimeouts.address()) === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        CloseHandle(_overlappedRead.hEvent);
        CloseHandle(_overlappedWrite.hEvent);
        throw ("GetCommTimeouts failed: " + ctypes.winLastError);
    }

    // Set Windows timeout values
    // timeouts is a tuple with the following items:
    // (ReadIntervalTimeout,ReadTotalTimeoutMultiplier,
    // ReadTotalTimeoutConstant,WriteTotalTimeoutMultiplier,
    // WriteTotalTimeoutConstant)
    let timeouts = new struct_COMMTIMEOUTS();
    if (timeout === null) {
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
    } else if (timeout === 0) {
        timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
    } else {
        timeouts.ReadIntervalTimeout = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = timeout * 1000;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
    }
    // TODO(rqou): Inter-char timeout not supported
    // TODO(rqou): Write timeout not supported
    SetCommTimeouts(hComPort, timeouts.address());

    // Setup the connection info.
    // Get state and modify it:
    let comDCB = new struct_DCB();
    if (+GetCommState(hComPort, comDCB.address()) === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        CloseHandle(_overlappedRead.hEvent);
        CloseHandle(_overlappedWrite.hEvent);
        throw ("GetCommState failed: " + ctypes.winLastError);
    }
    comDCB.BaudRate = baud;

    // TODO(rqou): Only 8 bits per char supported
    comDCB.ByteSize = 8;

    // TODO(rqou): Only no parity supported
    comDCB.Parity = NOPARITY;
    comDCB.f &= ~(fParity); // Disable Parity Check

    // TODO(rqou): Only 1 stop bit supported
    comDCB.StopBits = ONESTOPBIT;

    comDCB.f |= fBinary; // Enable Binary Transmission
    // Char. w/ Parity-Err are replaced with 0xff (if fErrorChar is set to TRUE)
    // TODO(rqou): RTS/CTS not supported
    comDCB.f = (comDCB.f & ~fRtsControl) | RTS_CONTROL_DISABLE;
    // TODO(rqou): DTR/DSR not supported
    comDCB.f = (comDCB.f & ~fDtrControl) | DTR_CONTROL_DISABLE;
    comDCB.f &= ~(fOutxCtsFlow);
    comDCB.f &= ~(fOutxDsrFlow);
    // TODO(rqou): Xon/Xoff not supported
    comDCB.f &= ~(fOutX);
    comDCB.f &= ~(fInX);
    comDCB.f &= ~(fNull);
    comDCB.f &= ~(fErrorChar);
    comDCB.f &= ~(fAbortOnError);
    comDCB.XonChar = XON;
    comDCB.XoffChar = XOFF;
    if (+SetCommState(hComPort, comDCB.address()) === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        CloseHandle(_overlappedRead.hEvent);
        CloseHandle(_overlappedWrite.hEvent);
        throw ("Cannot configure port, some setting was wrong. " +
               "Original message: " + ctypes.winLastError);
    }

    if (+PurgeComm(hComPort,
        PURGE_TXCLEAR | PURGE_TXABORT |
        PURGE_RXCLEAR | PURGE_RXABORT) === 0) {
        // Note, behavior slightly different from PySerial
        CloseHandle(hComPort);
        CloseHandle(_overlappedRead.hEvent);
        CloseHandle(_overlappedWrite.hEvent);
        throw ("PurgeComm failed: " + ctypes.winLastError);
    }

    let ret = {};
    ret.timeout = timeout;
    ret.hComPort = hComPort;
    ret._overlappedRead = _overlappedRead;
    ret._overlappedWrite = _overlappedWrite;
    ret._orgTimeouts = _orgTimeouts;

    // This is a hack to allow forwarding just enough data to make the reader
    // worker work. Among other things, closing the reader worker is probably
    // a bad idea because it won't recover the original timeouts.
    ret.fd = [hComPort.toSource(),
        _overlappedRead.hEvent.toSource(),
        _overlappedWrite.hEvent.toSource()];

    ret.close = serClose;
    ret.write = serWrite;
    ret.read = serRead;

    return ret;
};

exports.SerialPortClone = function(fd, timeout) {
    let _overlappedRead = new struct_OVERLAPPED();
    let _overlappedWrite = new struct_OVERLAPPED();

    /* jshint evil: true */
    _overlappedRead.hEvent = eval(fd[1]);
    _overlappedWrite.hEvent = eval(fd[2]);
    let hComPort = eval(fd[0]);

    let ret = {};
    ret.timeout = timeout;
    ret.hComPort = hComPort;
    ret._overlappedRead = _overlappedRead;
    ret._overlappedWrite = _overlappedWrite;
    ret._orgTimeouts = null;

    ret.fd = [hComPort.toSource(),
        _overlappedRead.hEvent.toSource(),
        _overlappedWrite.hEvent.toSource()];

    ret.close = function() {
        throw "Not recommended to close cloned serport object!";
    };
    ret.write = serWrite;
    ret.read = serRead;

    return ret;
};
