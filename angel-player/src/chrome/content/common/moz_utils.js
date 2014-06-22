/* jshint globalstrict: true */
"use strict";

/*
 * TODO(nikita): comment on what this file does
 *
 *
 */

const { Cu, Cc, Ci } = require("chrome");

// -- Debug module
// TODO(nikita): expose the mozilla preferences system instead?
let debugModule =
    Cu.import("chrome://angel-player/content/debug.js").debugModule;
exports.isDebugEnabled = debugModule.isDebugEnabled;
exports.toggleDebug = debugModule.toggleDebug;

// -- Application
const appStartup =
    Cc["@mozilla.org/toolkit/app-startup;1"].getService(Ci.nsIAppStartup);

exports.quit_application = function() {
    appStartup.quit(appStartup.eForceQuit);
};
