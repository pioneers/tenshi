/* jshint globalstrict: true */
"use strict";

const EventEmitter = require('tenshi/vendor-js/EventEmitter');
const ubjson = require('tenshi/vendor-js/ubjson');
const buffer = require('sdk/io/buffer');
const emcc_tools = require('tenshi/common/emcc_tools');
const xbee = require('tenshi/common/xbee');
const typpo = require('tenshi/common/typpo');

const MAX_PACKET_SIZE = 1024 * 1024;
const MAX_XBEE_PAYLOAD_SIZE = 256;
const SEND_INTERVAL = 100;

const UBJSON_PORT = typpo.get_const('NDL3_UBJSON_PORT');
const STRING_PORT = typpo.get_const('NDL3_STRING_PORT');
const CODE_PORT = typpo.get_const('NDL3_CODE_PORT');

try {
  var ndl3 = require('tenshi/vendor-js/ndl3');
} catch (_) {
  console.warn('Could not load ndl3.js. Did you run build.sh?');
}

function call(name) {
  var args = Array.prototype.slice.call(arguments, 1);
  var arg_ts = [];
  for (var i = 0; i < args.length; i++) {
    arg_ts.push('number');
  }
  return ndl3.ccall(name, 'number', arg_ts, args);
}

var Radio = function(address, serportObj) {
  EventEmitter.apply(this);
  this.connected = false;
  this.net = call('NDL3_new', 0, 0, 0);
  if (this.net === 0) {
    throw 'Could not allocate NDL3.';
  }
  call('NDL3_open', this.net, UBJSON_PORT);
  call('NDL3_open', this.net, STRING_PORT);
  call('NDL3_open', this.net, CODE_PORT);

  this._tick = send_L2.bind(this);
  setInterval(this._tick, SEND_INTERVAL);

  this.on('send_object', send_object.bind(this));
  this.on('send_string', send_string.bind(this));
  this.on('send_code', send_code.bind(this));
  this.on('data', recv_L2.bind(this));

  if (address !== undefined &&
      serportObj !== undefined) {
    this.connectXBee(address, serportObj);
  } else {
    this.address = null;
    this.serportObj = null;
  }

  // Print out every message in or out of the radio, for debugging
  // purposes.
  var events = ['data', 'object', 'string', 'code'];
  function make_evt_callback(evt) {
    return function (data) {
      console.log('radio: ', evt, ':', data);
    };
  }
  for (var e = 0; e < events.length; e++) {
    var evt = events[e];
    this.on(evt, make_evt_callback(evt));
    this.on('send_' + evt, make_evt_callback('send_' + evt));
  }
};

Radio.prototype = Object.create(EventEmitter.prototype);

Radio.prototype.isConnected = function () {
  return this.connected;
};

Radio.prototype.connectXBee = function (address, serportObj) {
  if (this.connected) {
    throw 'XBee has already been connected to radio.';
  }
  this.address = address;
  this.serportObj = serportObj;
  serportObj.setReadHandler(read_handler.bind(this));
  serportObj.setReadHandler = function () {
    throw 'Radio already attached to serial port.';
  };
  this._send_data = send_data.bind(this);
  this.on('send_data', this._send_data);
};

Radio.prototype.disconnectXBee = function () {
  if (!this.connected) {
    throw 'XBee not connected.';
  }
  this.off('send_data', this._send_data);
  delete this.serportObj.setReadHandler;
  this.serportObj.setReadHandler(null);
  this.address = null;
  this.serportObj = null;
};

function send_L2 () {
  /* jshint validthis: true */
  var out_size_ptr = call('malloc', emcc_tools.PTR_SIZE);
  var send_block = call('malloc', MAX_XBEE_PAYLOAD_SIZE);
  if (out_size_ptr === 0 || send_block === 0) {
    throw 'malloc failed';
  }
  call('NDL3_elapse_time', this.net, SEND_INTERVAL);
  call('NDL3_L2_pop', this.net, send_block, MAX_XBEE_PAYLOAD_SIZE, out_size_ptr);
  var length = emcc_tools.get_ptr(ndl3, out_size_ptr);
  // 1 + makes room for ident byte.
  var send_buf = buffer.Buffer(1 + length);
  send_buf[0] = typpo.get_const('NDL3_IDENT');
  emcc_tools.ptr_to_buffer(ndl3, send_block, length, send_buf, 1);
  call('free', out_size_ptr);
  call('free', send_block);
  var xbee_packet = xbee.createPacket(send_buf, this.address);
  this.emit('send_data', xbee_packet);
  throw_on_NDL3_error(this.net);
}

function read_handler(data) {
  /* jshint validthis: true */
  this.emit('data', data);
}

function recv_L2 (evt) {
  /* jshint validthis: true */
  var rxbuf = evt.data;
  var ptr = emcc_tools.buffer_to_ptr(ndl3, rxbuf);
  call('NDL3_L2_push', this.net, ptr, rxbuf.length);
  call('free', ptr);

  check_L3.apply(this);
  // Ignore errors due to bad packets.
  call('NDL3_pop_error', this.net);
}

function check_L3() {
  /* jshint validthis: true */
  check_port.apply(this, [UBJSON_PORT, function (buf) {
    /* jshint validthis: true */
    var self = this;
    ubjson.unpackBuffer(buf, function (object) {
      self.emit('object', object);
    });
  }]);
  check_port.apply(this, [STRING_PORT, function (buf) {
    this.emit('string', buf.toString());
  }]);
  check_port.apply(this, [CODE_PORT, function (buf) {
    this.emit('code', buf);
  }]);
}

function check_port(port, callback) {
  /* jshint validthis: true */
  var ptrs_buf = buffer.Buffer(emcc_tools.PTR_SIZE * 2);
  ptrs_buf.fill(0);
  var ptrs = emcc_tools.buffer_to_ptr(ndl3, ptrs_buf);
  call('NDL3_recv', this.net, port, ptrs, ptrs + emcc_tools.PTR_SIZE);
  var msg_ptr = emcc_tools.get_ptr(ndl3, ptrs);
  var size = emcc_tools.get_ptr(ndl3, ptrs + emcc_tools.PTR_SIZE);
  call('free', ptrs);
  if (emcc_tools.get_ptr(ndl3, ptrs) !== 0) {
    var buf = emcc_tools.ptr_to_buffer(ndl3, msg_ptr, size);
    callback.apply(this, [buf]);
  }
  throw_on_NDL3_error(this.net);
}

function throw_on_NDL3_error(net) {
  var err = call('NDL3_pop_error', net);
  if (err !== 0) {
    throw 'Error number ' + err + ' in NDL3.';
  }
}

Radio.prototype.send = function(data, type) {
  if (type === undefined) {
    type = 'object';
  }
  this.emit('send_' + type, data);
};

function send_object(object) {
  /* jshint validthis: true */
  var buf = buffer.Buffer(MAX_PACKET_SIZE);
  var length = ubjson.packToBufferSync(object, buf);
  send(this.net, UBJSON_PORT, buf, length);
}

function send_string(string) {
  /* jshint validthis: true */
  send(this.net, STRING_PORT, buffer.Buffer(string));
}

function send_code(buf) {
  /* jshint validthis: true */
  send(this.net, CODE_PORT, buf);
}

function send(net, port, buf, length) {
  var ptr = emcc_tools.buffer_to_ptr(ndl3, buf, length);
  call('NDL3_send', net, port, ptr, length);
}

function send_data(data) {
  /* jshint validthis: true */
  this.serportObj.write(data);
}


Radio.prototype.close = function() {
  call('NDL3_close', this.net, UBJSON_PORT);
  call('NDL3_close', this.net, STRING_PORT);
  call('NDL3_close', this.net, CODE_PORT);
  call('free', this.net);
  // TODO(kzentner): NDL3 provides no way of deleteing a transport.
  clearInterval(this._tick);
  if (this.connected) {
    this.disconnectXBee();
  }
};

exports.Radio = Radio;
