/* jshint globalstrict: true */
"use strict";

// This module contains various convenience functions for global preferences.
// Currently, this includes:
//  * Whether debug is enabled

const { Cu } = require("chrome");
const { Services } = Cu.import("resource://gre/modules/Services.jsm");

const DEBUG_MODE_PREF = "tenshi.enableDebug";

function isDebugEnabled() {
    return Services.prefs.getBoolPref(DEBUG_MODE_PREF);
}

// Returns the new state of the debug flag.
function toggleDebug() {
  let debugMode = isDebugEnabled();
  debugMode = !debugMode;
  Services.prefs.setBoolPref(DEBUG_MODE_PREF, debugMode);

  return debugMode;
}

exports.isDebugEnabled = isDebugEnabled;
exports.toggleDebug = toggleDebug;
