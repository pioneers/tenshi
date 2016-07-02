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

const typpo_module = require('tenshi/common/factory');
const xbee = require('tenshi/common/xbee');
const url = require('sdk/url');

const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
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

  let buf = xbee.createPacket(initial_packet, this.address);

  this.serportObj.write(buf);
};
