/* jshint globalstrict: true */
"use strict";

const { ChromeWorker } = require('chrome');
let typpo_module = require('tenshi/angelic/factory');
let url = require('jetpack/sdk/url');
let buffer = require('jetpack/sdk/io/buffer');
let Int64 = require('Int64.js');

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';
const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);

// TODO(rqou): Move this elsewhere
function computeXbeeChecksum(buf, start, len) {
    let sum = 0;

    for (let i = start; i < start + len; i++) {
        sum += buf[i];
    }

    return 0xFF - sum;
}

exports.sendPacketizedData = function(data) {
    // TODO(rqou): Proper lifecycle management for serial port interface
    let serportWorker = new ChromeWorker("common/serportWorker.js");
    // TODO(rqou): Don't hardcode
    serportWorker.postMessage({cmd: "open", data: "/dev/ttyUSB0"});

    serportWorker.onmessage = function(e) {
        let rxbuf = e.data;
        for (let i = 0; i < rxbuf.length; i++) {
            dump(rxbuf[i]);
            dump("\n");
        }

        dump("\n\n\n");
    };

    // Create initial packet
    let initial_packet = typpo.create('tenshi_bulk_start');
    initial_packet.set_slot('ident',
        typpo.get_const('TENSHI_NAIVE_BULK_START_IDENT'));
    // TODO(rqou): Meaningful stream IDs
    initial_packet.set_slot('stream_id', 0);
    initial_packet.set_slot('length', data.length);
    // The +1 allows the checksum to be crammed in the end
    let payloadbuf =
        buffer.Buffer(typpo.get_size('tenshi_bulk_start') + 1);
    payloadbuf.fill(0x00);
    initial_packet.write(payloadbuf);
    // Add XBee framing
    let xbee_tx_frame = typpo.create('xbee_tx64_header');
    xbee_tx_frame.set_slot('xbee_api_type',
        typpo.get_const('XBEE_API_TYPE_TX64'));
    // TODO(rqou): Use frameId?
    xbee_tx_frame.set_slot('frameId', 0);
    // TODO(rqou): Don't hardcode!!!
    xbee_tx_frame.set_slot('xbee_dest_addr', new Int64("0x0013A20040A580C4"));
    xbee_tx_frame.set_slot('options', 0);
    xbee_tx_frame.set_slot('data', payloadbuf);
    let xbee_payload = typpo.create('xbee_payload');
    xbee_payload.set_slot('tx64', xbee_tx_frame);
    let initial_packet_xbee = typpo.create('xbee_api_packet');
    initial_packet_xbee.set_slot('xbee_api_magic',
        typpo.get_const('XBEE_MAGIC'));
    initial_packet_xbee.set_slot('length',
        typpo.get_size('tenshi_bulk_start') +
        typpo.get_size('xbee_tx64_header'));
    initial_packet_xbee.set_slot('payload', xbee_payload);
    // TODO(rqou): Jank jank jank
    let buf = buffer.Buffer(
        typpo.get_size('xbee_api_packet') + payloadbuf.length);
    initial_packet_xbee.write(buf);
    // Note, this is kinda jank. Checksum is last byte. Getting the length
    // is also kinda borked due to the union.
    // TODO(rqou): Don't hardcode 3 here!
    buf[buf.length - 1] = computeXbeeChecksum(buf,
        3,
        buf.length - 1 - (3));
    serportWorker.postMessage({cmd: "write", data: buf});

    // TODO(rqou): Figure out when to close!
};
