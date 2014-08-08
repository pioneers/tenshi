const typpo_module = require('tenshi/common/factory');
const xbee = require('tenshi/common/xbee');
const buffer = require('sdk/io/buffer');
const url = require('sdk/url');

const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);

function buf_to_hex(buf) {
  let hex = '';
  for (var i = 0; i < buf.length; i++) {
    hex = hex + buf[i].toString(16);
  }
  return hex;
}


describe("XBee createPacket", function () {

  it ("should produce the right output from a buffer", function () {

    let buf = xbee.createPacket(buffer.Buffer('test payload'),
                                'deadbeefdeadbeef');
    // More or less manually calculated.
    let e_hex = '7e01700deadbeefdeadbeef074657374207061796c6f6164c5';
    expect(buf_to_hex(buf)).toEqual(e_hex);

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
    expect(buf_to_hex(buf)).toEqual(e_hex);

  });

  it ("should extract payloads correctly", function () {
    let input = buffer.Buffer([126, 0, 22, 128, 0, 19, 162, 0, 64, 165, 128,
                              196, 75, 0, 100, 240, 128, 78, 2, 3, 0, 60, 0, 0,
                              0, 243]);
    let output = xbee.extractPayload(input);
    let e_hex = '64f0804e2303c000';
    expect(buf_to_hex(output)).toEqual(e_hex);
  });

});
