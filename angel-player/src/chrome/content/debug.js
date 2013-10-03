Components.utils.import("resource://gre/modules/Services.jsm");

var EXPORTED_SYMBOLS = ["debugModule"];

var debugModule = {};

var DEBUG_MODE_PREF = "tenshi.enableDebug";
var debugMode = false;

debugModule.init = function() {
    debugMode = Services.prefs.getBoolPref(DEBUG_MODE_PREF);
};

debugModule.isDebugEnabled = function() {
    return debugMode;
};

debugModule.toggleDebug = function() {
    debugMode = !debugMode;
    Services.prefs.setBoolPref(DEBUG_MODE_PREF, debugMode);
};
