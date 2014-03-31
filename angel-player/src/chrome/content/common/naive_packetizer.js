/* jshint globalstrict: true */
"use strict";

const { ChromeWorker } = require('chrome');
let typpo_module = require('tenshi/angelic/factory');
let url = require('jetpack/sdk/url');
let buffer = require('jetpack/sdk/io/buffer');

const FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(FRAMING_YAML_FILE));

exports.sendPacketizedData = function(data) {

    // TODO(rqou): Proper lifecycle management for serial port interface
    let serportWorker = new ChromeWorker("common/serportWorker.js");
    // TODO(rqou): Don't hardcode
    serportWorker.postMessage({cmd: "open", data: "/dev/ttyUSB0"});

    serportWorker.onmessage = function(e) {
        let asdfbuf = e.data;
        for (let i = 0; i < asdfbuf.length; i++) {
            dump(asdfbuf[i]);
            dump("\n");
        }

        dump("\n\n\n");
    };

    let initial_packet = typpo.create('tenshi_bulk_start');
    initial_packet.set_slot('ident',
        typpo.get_const('TENSHI_NAIVE_BULK_START_IDENT'));
    // TODO(rqou): Meaningful stream IDs
    initial_packet.set_slot('stream_id', 0);
    initial_packet.set_slot('length', data.length);
    let buf = buffer.Buffer(typpo.get_size('tenshi_bulk_start'));
    initial_packet.write(buf);
    serportWorker.postMessage({cmd: "write", data: buf});

    // TODO(rqou): Figure out when to close!
};
