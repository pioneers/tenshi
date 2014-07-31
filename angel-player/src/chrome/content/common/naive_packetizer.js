/* jshint globalstrict: true */
"use strict";

const typpo_module = require('tenshi/common/factory');
const url = require('sdk/url');
const buffer = require('sdk/io/buffer');
const global_state = require('tenshi/common/global_state');
const xbee = require('tenshi/common/xbee');
const ioports = require('tenshi/welcome/ioports');

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';
const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);

exports.sendPacketizedData = function(data) {
    let serportObj = global_state.get('serial_port_object');
    if (!serportObj) {
        throw "Serial port not open!";
    }

    let ROBOT = ioports.get_xbee_addr();
    if (!ROBOT) {
        throw "No radio address set!";
    }

    let main_radio = global_state.get('main_radio');
    if (!main_radio) {
        throw "Could not get main radio.";
    }

    main_radio.on('data', function (rxbuf) {
        let rx_packet =
            typpo.read('xbee_api_packet', buffer.Buffer(rxbuf)).unwrap();
        let checksum = xbee.computeChecksum(
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

            let buf = xbee.createPacket(initial_packet, ROBOT);

            dump("Sent bytes " + chunkreq.start_addr + " to " +
                chunkreq.end_addr + "\n");

            serportObj.write(buf);
        }
        else if (rx_packet.payload.rx64.data[0] ===
            typpo.get_const('TENSHI_NAIVE_BULK_STOP_IDENT')) {
            dump("DONE DONE DONE!\n");

            // TODO(rqou): Do something here? Waiting for done?
        }
    });

    // Create initial packet
    let initial_packet = typpo.create('tenshi_bulk_start');
    initial_packet.set_slot('ident',
        typpo.get_const('TENSHI_NAIVE_BULK_START_IDENT'));
    // TODO(rqou): Meaningful stream IDs
    initial_packet.set_slot('stream_id', 0);
    initial_packet.set_slot('length', data.length);

    let buf = xbee.createPacket(initial_packet, ROBOT);

    serportObj.write(buf);
};
