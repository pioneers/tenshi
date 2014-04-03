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

    sum = sum & 0xFF;

    return 0xFF - sum;
}

exports.sendPacketizedData = function(data) {
    // TODO(rqou): Proper lifecycle management for serial port interface
    let serportWorker = new ChromeWorker("common/serportWorker.js");
    // TODO(rqou): Don't hardcode
    serportWorker.postMessage({cmd: "open", data: "/dev/ttyUSB0"});

    const ROBOT = "0x0013A20040A580C4";

    serportWorker.onmessage = function(e) {
        let rxbuf = e.data;
        let rx_packet =
            typpo.read('xbee_api_packet', buffer.Buffer(rxbuf)).unwrap();
        let checksum = computeXbeeChecksum(
            rx_packet.payload.bytes, 0, rx_packet.length);
        if (rx_packet.payload.bytes[rx_packet.length] !== checksum) {
            dump("Bad checksum!\n");
            return;
        }
        if (rx_packet.xbee_api_magic != typpo.get_const('XBEE_MAGIC')) {
            dump("Bad XBee magic!\n");
            return;
        }
        if (rx_packet.payload.xbee_api_type !=
            typpo.get_const('XBEE_API_TYPE_RX64')) {
            dump("Ignoring unknown packet back: " +
                rx_packet.payload.xbee_api_type.toString(16) + "\n");
            return;
        }

        if (rx_packet.payload.rx64.data[0] === 
            typpo.get_const('TENSHI_NAIVE_BULK_CHUNKREQ_IDENT')) {
            let chunkreq = typpo.read('tenshi_bulk_chunkreq',
                buffer.Buffer(rx_packet.payload.rx64.data)).unwrap();
            // TODO(rqou): Stream id?

            // Create reply packet
            let initial_packet = typpo.create('tenshi_bulk_chunk');
            initial_packet.set_slot('ident',
                typpo.get_const('TENSHI_NAIVE_BULK_CHUNK_IDENT'));
            // TODO(rqou): Meaningful stream IDs
            initial_packet.set_slot('stream_id', 0);
            initial_packet.set_slot('start_addr', chunkreq.start_addr);
            initial_packet.set_slot('end_addr', chunkreq.end_addr);
            let chunklen = chunkreq.end_addr - chunkreq.start_addr;
            // TODO(rqou): This isn't efficient?
            let replyChunk = buffer.Buffer(chunklen);
            for (let i = chunkreq.start_addr; i < chunkreq.end_addr; i++) {
                replyChunk[i - chunkreq.start_addr] = data[i]; 
            }
            initial_packet.set_slot('data', replyChunk);
            // The +1 allows the checksum to be crammed in the end
            let payloadbuf =
                buffer.Buffer(
                    typpo.get_size('tenshi_bulk_chunk') + chunklen + 1);
            payloadbuf.fill(0x00);
            initial_packet.write(payloadbuf);
            // Add XBee framing
            let xbee_tx_frame = typpo.create('xbee_tx64_header');
            xbee_tx_frame.set_slot('xbee_api_type',
                typpo.get_const('XBEE_API_TYPE_TX64'));
            // TODO(rqou): Use frameId?
            xbee_tx_frame.set_slot('frameId', 0);
            // TODO(rqou): Don't hardcode!!!
            xbee_tx_frame.set_slot('xbee_dest_addr', new Int64(ROBOT));
            xbee_tx_frame.set_slot('options', 0);
            xbee_tx_frame.set_slot('data', payloadbuf);
            let xbee_payload = typpo.create('xbee_payload');
            xbee_payload.set_slot('tx64', xbee_tx_frame);
            let initial_packet_xbee = typpo.create('xbee_api_packet');
            initial_packet_xbee.set_slot('xbee_api_magic',
                typpo.get_const('XBEE_MAGIC'));
            initial_packet_xbee.set_slot('length',
                typpo.get_size('tenshi_bulk_chunk') +
                typpo.get_size('xbee_tx64_header') +
                chunklen);
            initial_packet_xbee.set_slot('payload', xbee_payload);
            // TODO(rqou): Jank jank jank
            let buf = buffer.Buffer(
                typpo.get_size('xbee_api_packet') + payloadbuf.length);
            initial_packet_xbee.write(buf);
            // Note, this is kinda jank. Checksum is last byte. Getting the
            // length is also kinda borked due to the union.
            // TODO(rqou): Don't hardcode 3 here!
            buf[buf.length - 1] = computeXbeeChecksum(buf,
                3,
                buf.length - 1 - (3));
            dump("Sent bytes " + chunkreq.start_addr + " to " +
                chunkreq.end_addr + "\n");
            serportWorker.postMessage({cmd: "write", data: buf});
        }
        else if (rx_packet.payload.rx64.data[0] === 
            typpo.get_const('TENSHI_NAIVE_BULK_STOP_IDENT')) {
            dump("DONE DONE DONE!\n");
            serportWorker.postMessage({cmd: "close"});
        }
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
    xbee_tx_frame.set_slot('xbee_dest_addr', new Int64(ROBOT));
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
};
