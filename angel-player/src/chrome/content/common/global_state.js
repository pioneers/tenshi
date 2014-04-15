/* jshint globalstrict: true */
"use strict";

// Nothing is in here by itself; things get shoved into this module to be
// stored across pages.

let state = {};

// TODO(rqou): Majorly refactor global state

exports.set = function(key, val) {
    state[key] = val;
};

exports.get = function(key) {
    return state[key];
};
