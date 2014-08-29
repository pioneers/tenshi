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

const EventEmitter = require('tenshi/vendor-js/EventEmitter');
const typpo_module = require('tenshi/common/factory');
const Int64 = require('Int64.js');
const buffer = require('sdk/io/buffer');
const url = require('sdk/url');

const MAX_XBEE_PACKET_SIZE = 0x10000;

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);


var Accumulator = function(serportObj) {
  EventEmitter.apply(this);
  this.buf = buffer.Buffer(MAX_XBEE_PACKET_SIZE);
  this.offset = 0;
  this._on_recv = on_recv.bind(this);
  this.serportObj = serportObj;
  serportObj.on('data', this._on_recv);
};

exports.Accumulator = Accumulator;

Accumulator.prototype = Object.create(EventEmitter.prototype);

Accumulator.prototype.destroy = function () {
  this.serportObj.off('data', this._on_recv);
};

var on_recv = function (data) {
  data.copy(this.buf, this.offset, 0, data.length);
  this.offset += data.length;
  var buf = this.buf.slice(0, this.offset);
  if (exports.isFullPacket(buf)) {
    this.buf = buffer.Buffer(MAX_XBEE_PACKET_SIZE);
    this.offset = 0;
    this.emit('data', buf);
  }
};


// This module contains utilities for using an XBee radio.
// TODO(kzentner): Move other, duplicated xbee functionality here.

exports.computeChecksum = function (buf, start, len) {
    if (start === undefined) {
      start = typpo.get_size('xbee_api_packet', true) -
              typpo.get_size('xbee_tx64_header', true);
    }
    if (len === undefined) {
      // -1 for checksum byte at the end.
      len = buf.length - start - 1;
    }
    var sum = 0;

    for (var i = start; i < start + len; i++) {
        sum += buf[i];
    }

    sum = sum & 0xFF;

    return 0xFF - sum;
};

// Creates a full XBee packet.
// payload should be an object returned from factory.js, or a buffer.
// Address should be a hexadecimal string.

exports.createPacket = function (payload, address) {
  let payload_length = 0;
  if (payload instanceof buffer.Buffer) {
    payload_length = payload.length;
  } else if (payload.type) {
    payload_length = payload.get_size();
    // We're going to store payload into a uint8_t[] below, so cast it.
    payload = payload.cast();
  } else {
    throw "Could not determine type of XBee payload.";
  }
  let xbee_tx_frame = typpo.wrap(typpo.get_type('xbee_tx64_header'), {
    xbee_api_type: typpo.get_const('XBEE_API_TYPE_TX64'),
    frameId: 0,
    xbee_dest_addr: new Int64("0x" + address),
    options: 0,
    data: payload
  });

  let xbee_payload = typpo.wrap(typpo.get_type('xbee_payload'), {
    tx64: xbee_tx_frame
  });

  let xbee_packet = typpo.wrap(typpo.get_type('xbee_api_packet'), {
    xbee_api_magic: typpo.get_const('XBEE_MAGIC'),
    length: typpo.get_size('xbee_tx64_header', true) + payload_length,
    payload: xbee_payload
  });

  // The +1 allows the checksum to be crammed in the end

  let buf = buffer.Buffer(typpo.get_size('xbee_api_packet', true) + payload_length + 1);

  xbee_packet.write(buf);

  // Note, this is kinda jank.
  buf[buf.length - 1] = exports.computeChecksum(buf);

  return buf;
};

// Returns true if buf contains *exactly* a single valid XBee packet.
// Extra space at the end of the buffer is not allowed.
exports.isFullPacket = function (buf) {
  var header_size = typpo.get_size('xbee_api_packet', true);
  if (buf.length < header_size) {
    return false;
  }
  try {
    var xbee_packet = typpo.read('xbee_api_packet', buf);
    var payload_header_size = typpo.get_size('xbee_tx64_header', true);
    var length = xbee_packet.get_slot('length').val;
    var checksum_size = 1;
    if (buf.length === length + header_size + checksum_size - payload_header_size) {
      var checksum = exports.computeChecksum(buf);
      if (checksum === buf[buf.length - 1]) {
        return true;
      }
    }
  } catch (_) {
    // The packet caused an error, so it was almost certainly corrupt (and thus
    // not a full packet).
  }
  return false;
};

exports.extractPayload = function (rxPacket) {
  let api_packet = typpo.read('xbee_api_packet', rxPacket);
  let payload = api_packet.get_slot('payload');
  let api_type = payload.get_slot('xbee_api_type').unwrap();
  if (api_type !== typpo.get_const('XBEE_API_TYPE_RX64')) {
    throw "Attempt to extract the payload from a non-rx64 packet.";
  }
  let rx = api_packet.get_slot('payload').get_slot('rx64');
  let data = rx.get_slot('data').unwrap();

  // Don't ouput checksum.
  return data.slice(0, -1);
};
