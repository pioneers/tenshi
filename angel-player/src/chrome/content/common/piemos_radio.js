/* jshint globalstrict: true */
"use strict";

const typpo_module = require('tenshi/common/factory');
const xbee = require('tenshi/common/xbee');
const url = require('jetpack/sdk/url');
const Int64 = require('Int64.js');
const buffer = require('jetpack/sdk/io/buffer');

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';
const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);

exports.Radio = function(address, serial_port_object) {
  this.address = address;
  this.serportObj = serial_port_object;
  if (!this.serportObj) {
    throw 'Could not find XBee serial port!';
  }
  if (!this.address) {
    throw 'Robot address not set!';
  }
};

exports.Radio.prototype.send = function(data) {

  // This radio module only supports the aged PiEMOS radio format.
  // Therefore, only the following two fields are supported.

  for (let field in data) {
    if (field != 'PiELESDigitalVals' && field != 'PiELESAnalogVals') {
      throw ('Keys besides PiELESDigitalVals and PiELESAnalogVals are ' +
             'not supported in this protocol.');
    }
  }

  let PiELESDigitalVals = data.PiELESDigitalVals || [];
  let PiELESAnalogVals = data.PiELESAnalogVals || [];

  let digitalBitfield = 0;
  for (let i = 0; i < 8; i++) {
      if (PiELESDigitalVals[i]) {
          digitalBitfield |= (1 << i);
      }
  }

  // Create initial packet
  let initial_packet = typpo.create('pier_incomingdata');
  initial_packet.set_slot('ident',
      typpo.get_const('PIER_INCOMINGDATA_IDENT'));
  // TODO(rqou): Meaningful flags?
  initial_packet.set_slot('fieldtime', 0);
  initial_packet.set_slot('flags', 0);
  initial_packet.set_slot('analog', PiELESAnalogVals);
  initial_packet.set_slot('digital', digitalBitfield);
  // The +1 allows the checksum to be crammed in the end
  let payloadbuf =
      buffer.Buffer(typpo.get_size('pier_incomingdata') + 1);
  payloadbuf.fill(0x00);
  initial_packet.write(payloadbuf);
  // Add XBee framing
  let xbee_tx_frame = typpo.create('xbee_tx64_header');
  xbee_tx_frame.set_slot('xbee_api_type',
      typpo.get_const('XBEE_API_TYPE_TX64'));
  // TODO(rqou): Use frameId?
  xbee_tx_frame.set_slot('frameId', 0);
  xbee_tx_frame.set_slot('xbee_dest_addr', new Int64("0x" + this.address));
  xbee_tx_frame.set_slot('options', 0);
  xbee_tx_frame.set_slot('data', payloadbuf);
  let xbee_payload = typpo.create('xbee_payload');
  xbee_payload.set_slot('tx64', xbee_tx_frame);
  let initial_packet_xbee = typpo.create('xbee_api_packet');
  initial_packet_xbee.set_slot('xbee_api_magic',
      typpo.get_const('XBEE_MAGIC'));
  initial_packet_xbee.set_slot('length',
      typpo.get_size('pier_incomingdata') +
      typpo.get_size('xbee_tx64_header'));
  initial_packet_xbee.set_slot('payload', xbee_payload);
  // TODO(rqou): Jank jank jank
  let buf = buffer.Buffer(
      typpo.get_size('xbee_api_packet') + payloadbuf.length);
  initial_packet_xbee.write(buf);
  // Note, this is kinda jank. Checksum is last byte. Getting the length
  // is also kinda borked due to the union.
  // TODO(rqou): Don't hardcode 3 here!
  buf[buf.length - 1] = xbee.computeChecksum(buf,
      3,
      buf.length - 1 - (3));

  this.serportObj.write(buf);
};
