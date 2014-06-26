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

const typpo_module = require('tenshi/common/factory');
const xbee = require('tenshi/common/xbee');
const buffer = require('sdk/io/buffer');
const url = require('sdk/url');

const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);


describe("XBee createPacket", function () {

  it ("should produce the right output from a buffer", function () {

    let buf = xbee.createPacket(buffer.Buffer('test payload'),
                                'deadbeefdeadbeef');
    // More or less manually calculated.
    let e_hex = '7e01700deadbeefdeadbeef074657374207061796c6f6164c5';
    let hex = '';
    for (var i = 0; i < buf.length; i++) {
      hex = hex + buf[i].toString(16);
    }
    expect(hex).toEqual(e_hex);

  });

  it ("should produce the right output from a typpo object", function () {

    let data = typpo.create('pier_incomingdata');
    data.set_slot('ident',
        typpo.get_const('PIER_INCOMINGDATA_IDENT'));
    // TODO(rqou): Meaningful flags?
    data.set_slot('fieldtime', 0);
    data.set_slot('flags', 0);
    data.set_slot('analog', [127, 127, 127, 127, 127, 127, 127]);
    data.set_slot('digital', 0);

    let buf = xbee.createPacket(data,
                                'deadbeefdeadbeef');
    // More or less manually calculated.
    let e_hex = '7e01600deadbeefdeadbeef0fe007f7f7f7f7f7f7f018';
    let hex = '';
    for (var i = 0; i < buf.length; i++) {
      hex = hex + buf[i].toString(16);
    }
    expect(hex).toEqual(e_hex);

  });

});
