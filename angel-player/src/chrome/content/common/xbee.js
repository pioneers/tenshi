/* jshint globalstrict: true */
"use strict";

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
