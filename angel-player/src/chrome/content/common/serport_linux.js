/* jshint globalstrict: true */
"use strict";

// Handles serial ports on Linux only. Very inspired by pySerial.

const { Cu } = require("chrome");
Cu.import('resource://gre/modules/ctypes.jsm');

let _doneBasicInit = false;

const LIBC_PATH = 'libc.so.6';

let libc;
let open;
let read;
let write;
let ioctl;
let close;
let tcgetattr;
let tcsetattr;
let cfsetispeed;
let cfsetospeed;
let select;

const struct_termios = new ctypes.StructType(
    'termios',
    [
        { 'c_iflag': ctypes.unsigned_int },
        { 'c_oflag': ctypes.unsigned_int },
        { 'c_cflag': ctypes.unsigned_int },
        { 'c_lflag': ctypes.unsigned_int },
        { 'c_line': ctypes.unsigned_char },
        { 'c_cc': new ctypes.ArrayType(ctypes.unsigned_char, 32) },
        { 'c_ispeed': ctypes.unsigned_int },
        { 'c_ospeed': ctypes.unsigned_int },
    ]
    );

const struct_timeval = new ctypes.StructType(
    'timeval',
    [
        { 'tv_sec': ctypes.long },
        { 'tv_usec': ctypes.long },
    ]
    );

const O_RDWR        = 0x0002;
const O_NOCTTY      = 0x0400;
const O_NONBLOCK    = 0x2000;

const CREAD         = 128;
const CLOCAL        = 2048;

const ISIG          = 1;
const ICANON        = 2;
const ECHO          = 8;
const ECHOE         = 16;
const ECHOK         = 32;
const ECHONL        = 64;
const IEXTEN        = 32768;

const OPOST         = 1;

const IGNBRK        = 1;
const PARMRK        = 8;
const INLCR         = 64;
const IGNCR         = 128;
const ICRNL         = 256;
const IUCLC         = 512;

const CSIZE         = 48;
const   CS5         = 0;
const   CS6         = 16;
const   CS7         = 32;
const   CS8         = 48;
const CSTOPB        = 64;

const INPCK         = 16;
const ISTRIP        = 32;
const PARENB        = 256;
const PARODD        = 512;

const IXON          = 1024;
const IXANY         = 2048;
const IXOFF         = 4096;

const CRTSCTS       = 2147483648;

const VTIME         = 5;
const VMIN          = 6;

const TCSANOW       = 0;

const EAGAIN        = 11;

const baudrate_constants = {
    0:          0, // hang up
    50:         1,
    75:         2,
    110:        3,
    134:        4,
    150:        5,
    200:        6,
    300:        7,
    600:        8,
    1200:       9,
    1800:       10,
    2400:       11,
    4800:       12,
    9600:       13,
    19200:      14,
    38400:      15,
    57600:      4097,
    115200:     4098,
    230400:     4099,
    460800:     4100,
    500000:     4101,
    576000:     4102,
    921600:     4103,
    1000000:    4104,
    1152000:    4105,
    1500000:    4106,
    2000000:    4107,
    2500000:    4108,
    3000000:    4109,
    3500000:    4110,
    4000000:    4111
};

exports.init = function() {
    if (_doneBasicInit) {
        return;
    }

    libc = ctypes.open(LIBC_PATH);

    // Get functions we need to work with I/O ports. Note that anything that
    // would normally be a void* is declared as uint8_t*
    open = libc.declare(
        'open',
        ctypes.default_abi,
        ctypes.int,

        ctypes.char.ptr,
        ctypes.int
        );
    read = libc.declare(
        'read',
        ctypes.default_abi,
        ctypes.ssize_t,

        ctypes.int,
        ctypes.uint8_t.ptr,
        ctypes.size_t
        );
    write = libc.declare(
        'write',
        ctypes.default_abi,
        ctypes.ssize_t,

        ctypes.int,
        ctypes.uint8_t.ptr,
        ctypes.size_t
        );
    ioctl = libc.declare(
        'ioctl',
        ctypes.default_abi,
        ctypes.int,

        ctypes.int,
        ctypes.unsigned_long,
        ctypes.uint8_t.ptr
        );
    close = libc.declare(
        'close',
        ctypes.default_abi,
        ctypes.int,

        ctypes.int
        );
    tcgetattr = libc.declare(
        'tcgetattr',
        ctypes.default_abi,
        ctypes.int,

        ctypes.int,
        struct_termios.ptr
        );
    tcsetattr = libc.declare(
        'tcsetattr',
        ctypes.default_abi,
        ctypes.int,

        ctypes.int,
        ctypes.int,
        struct_termios.ptr
        );
    select = libc.declare(
        'select',
        ctypes.default_abi,
        ctypes.int,

        ctypes.int,
        ctypes.uint8_t.ptr,     // These are supposed to be fd_set*, but on
        ctypes.uint8_t.ptr,     // Linux at least fd_set is just an array of
        ctypes.uint8_t.ptr,     // bitfields that we handle manually.
        struct_timeval.ptr
        );
    cfsetispeed = libc.declare(
        'cfsetispeed',
        ctypes.default_abi,
        ctypes.int,

        struct_termios.ptr,
        ctypes.unsigned_int
        );
    cfsetospeed = libc.declare(
        'cfsetospeed',
        ctypes.default_abi,
        ctypes.int,

        struct_termios.ptr,
        ctypes.unsigned_int
        );

    _doneBasicInit = true;
};

function serClose() {
    /* jshint validthis: true */
    // TODO(rqou): Figure out why this is confusing jshint.
    close(this.fd);
}

function fd_set_get_idx(fd) {
    // Unfortunately, we actually have an array of long ints, which is
    // a) platform dependent and b) not handled by typed arrays. We manually
    // figure out which byte we should be in. We assume a 64-bit platform
    // that is little endian (aka x86_64 linux).
    let elem64 = Math.floor(fd / 64);
    let bitpos64 = fd % 64;
    let elem8 = elem64 * 8;
    let bitpos8 = bitpos64;
    if (bitpos8 >= 8) {     // 8
        bitpos8 -= 8;
        elem8++;
    }
    if (bitpos8 >= 8) {     // 16
        bitpos8 -= 8;
        elem8++;
    }
    if (bitpos8 >= 8) {     // 24
        bitpos8 -= 8;
        elem8++;
    }
    if (bitpos8 >= 8) {     // 32
        bitpos8 -= 8;
        elem8++;
    }
    if (bitpos8 >= 8) {     // 40
        bitpos8 -= 8;
        elem8++;
    }
    if (bitpos8 >= 8) {     // 48
        bitpos8 -= 8;
        elem8++;
    }
    if (bitpos8 >= 8) {     // 56
        bitpos8 -= 8;
        elem8++;
    }

    return {'elem8': elem8, 'bitpos8': bitpos8};
}

function fd_set_set(fdset, fd) {
    let { elem8, bitpos8 } = fd_set_get_idx(fd);
    fdset[elem8] = 1 << bitpos8;
}

function fd_set_isset(fdset, fd) {
    let { elem8, bitpos8 } = fd_set_get_idx(fd);
    return !!(fdset[elem8] & (1 << bitpos8));
}

// Expects data to be a Uint8Array
function serWrite(data) {
    /* jshint validthis: true */

    let _d = new ctypes.uint8_t.ptr(data.buffer);
    let d = ctypes.cast(_d, ctypes.uint8_t.array(data.length).ptr);
    let tx_len = data.length;
    let bufidx = 0;
    // TODO(rqou): Timeouts not supported yet
    while (tx_len > 0) {
        let n = +write(this.fd, d.contents.addressOfElement(bufidx), tx_len);
        if (n === -1) {
            if (ctypes.errno != EAGAIN) {
                throw ("Write failed, errno = " + ctypes.errno);
            }
            continue;
        }
        // wait for write operation
        let fdset = new Uint8Array(128);
        for (let i = 0; i < fdset.length; i++) {
            fdset[i] = 0;
        }
        fd_set_set(fdset, this.fd);

        let ret = +select(this.fd + 1, null, fdset, null, null);
        if (ret === -1) {
            throw ("Write failed (select), errno = " + ctypes.errno);
        }
        let ready = fd_set_isset(fdset, this.fd);
        if (!ready) {
            throw "Write failed (select)";
        }
        bufidx += n;
        tx_len -= n;
    }
    return tx_len;
}

// select based implementation, proved to work on many systems
function serRead(size) {
    /* jshint validthis: true */

    if (typeof(size) === 'undefined') size = 1;

    let readbuf = new Uint8Array(size);
    let _buf = new ctypes.uint8_t.ptr(readbuf.buffer);
    let buf = ctypes.cast(_buf, ctypes.uint8_t.array(size).ptr);
    let readlength = 0;
    while (readlength < size) {
        let fdset = new Uint8Array(128);
        for (let i = 0; i < fdset.length; i++) {
            fdset[i] = 0;
        }
        fd_set_set(fdset, this.fd);

        let timeoutStruct;
        if (this.timeout === null) {
            timeoutStruct = null;
        } else {
            timeoutStruct = new struct_timeval();
            // Note: not the full range of timeouts works due to limited range
            // of double.
            timeoutStruct.tv_sec = Math.floor(this.timeout);
            timeoutStruct.tv_usec = Math.floor((this.timeout % 1) * 1000000);
        }

        let ret = +select(this.fd + 1, fdset, null, null,
            timeoutStruct ? timeoutStruct.address() : null);
        if (ret === -1) {
            throw ("read failed, errno = " + ctypes.errno);
        }
        let ready = fd_set_isset(fdset, this.fd);
        if (!ready) {
            break;  // timeout
        }
        ret = +read(this.fd, buf.contents.addressOfElement(readlength),
            size-readlength);
        if (ret === -1) {
            if (ctypes.errno != EAGAIN) {
                // ignore EAGAIN errors. all other errors are shown
                throw ("read failed, errno = " + ctypes.errno);
            }
            continue;
        }
        // read should always return some data as select reported it was
        // ready to read when we get to this point.
        if (ret === 0) {
            // Disconnected devices, at least on Linux, show the
            // behavior that they are always ready to read immediately
            // but reading returns nothing.
            throw ('device reports readiness to read but returned no data' +
                ' (device disconnected or multiple access on port?)');
        }
        readlength += ret;
    }
    // TODO(rqou): This is kinda jank
    if (readlength === size) {
        return readbuf;
    }
    // Otherwise, we read an incomplete buffer.
    let trimmed_ret = new Uint8Array(readlength);
    for (let i = 0; i < readlength; i++) {
        trimmed_ret[i] = readbuf[i];
    }
    return trimmed_ret;
}

exports.SerialPortOpen = function(port, baud, timeout) {

    if (typeof(timeout) === 'undefined') timeout = null;

    let fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        throw ("Error opening port " + port + ": errno = " + ctypes.errno);
    }

    let termios_data = new struct_termios();
    if (+tcgetattr(fd, termios_data.address()) !== 0) {
        throw ("Error configuring port " + port + ": errno = " + ctypes.errno);
    }

    // set up raw mode / no echo / binary
    termios_data.c_cflag |= (CLOCAL|CREAD);
    termios_data.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ISIG|IEXTEN);
    // TODO(rqou): Do we need the ECHOCTL/ECHOKE? Comment says for NetBSD.
    termios_data.c_oflag &= ~(OPOST);
    termios_data.c_iflag &= ~(INLCR|IGNCR|ICRNL|IGNBRK|IUCLC|PARMRK);

    // setup baud rate
    cfsetispeed(termios_data.address(), baudrate_constants[baud]);
    cfsetospeed(termios_data.address(), baudrate_constants[baud]);
    // TODO(rqou): Support custom baud rates.

    // setup char len
    // TODO(rqou): Hardcoded to 8
    termios_data.c_cflag &= ~CSIZE;
    termios_data.c_cflag |= CS8;

    // setup stopbits
    // TODO(rqou): Hardcoded to 1
    termios_data.c_cflag &= ~(CSTOPB);

    // setup parity
    // TODO(rqou): Hardcoded to n
    termios_data.c_iflag &= ~(INPCK|ISTRIP);
    termios_data.c_cflag &= ~(PARENB|PARODD);

    // setup flow control
    // TODO(rqou): Hardcoded to all off
    // xonxoff
    termios_data.c_iflag &= ~(IXON|IXOFF|IXANY);

    // rtscts
    termios_data.c_cflag &= ~(CRTSCTS);
    // XXX should there be a warning if setting up rtscts (and xonxoff etc)
    // fails??

    // TODO(rqou): vmin, vtime hardcoded
    termios_data.c_cc[VMIN] = 0;
    termios_data.c_cc[VTIME] = 0;
    if (+tcsetattr(fd, TCSANOW, termios_data.address()) !== 0) {
        throw ("Error configuring port " + port + ": errno = " + ctypes.errno);
    }

    let ret = {};
    ret.fd = fd;
    ret.timeout = timeout;
    ret.write = serWrite;
    ret.read = serRead;
    ret.close = serClose;

    return ret;
};
