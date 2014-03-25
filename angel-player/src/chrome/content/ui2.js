/* jshint globalstrict: true */
"use strict";

const { ChromeWorker } = require('chrome');
let serportWorker = null;

exports.init = function(_window) {
    serportWorker = new ChromeWorker("serportTest.js");
};
