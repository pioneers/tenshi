/* jshint globalstrict: true */
"use strict";

const { Cu, Cc, Ci } = require("chrome");

const console =
    Cu.import("resource://gre/modules/devtools/Console.jsm").console;

//TODO(nikita): Are their more functions in the console?
exports.log = console.log;
