/* jshint globalstrict: true */
"use strict";

const robot_application = require('tenshi/common/robot_application');

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

exports.set('robot_application', robot_application.CreateEmptyRobotApplication());
