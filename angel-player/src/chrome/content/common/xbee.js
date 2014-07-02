/* jshint globalstrict: true */
"use strict";

const typpo_module = require('tenshi/common/factory');
const Int64 = require('Int64.js');
const buffer = require('sdk/io/buffer');
const url = require('sdk/url');

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);


// This module contains utilities for using an XBee radio.
// TODO(kzentner): Move other, duplicated xbee functionality here.

exports.computeChecksum = function (buf, start, len) {
    let sum = 0;

    for (let i = start; i < start + len; i++) {
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
    payload_length = typpo.get_size(payload.type);
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
    length: typpo.get_size('xbee_tx64_header') + payload_length,
    payload: xbee_payload
  });

  // The +1 allows the checksum to be crammed in the end

  let buf = buffer.Buffer(typpo.get_size('xbee_api_packet') + payload_length + 1);

  xbee_packet.write(buf);

  // Note, this is kinda jank. Checksum is last byte. Getting the length
  // is also kinda borked due to the union.
  // TODO(rqou): Don't hardcode 3 here!
  // Note: 3 is because the checksum is defined to skip the magic and length 
  // fields (a total of 3 bytes).
  buf[buf.length - 1] = exports.computeChecksum(buf,
      3,
      buf.length - 1 - (3));

  return buf;
};
