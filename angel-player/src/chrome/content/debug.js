Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
Components.utils.import("resource://gre/modules/Services.jsm");

var EXPORTED_SYMBOLS = ["debugModule"];

var debugModule = {};

var DEBUG_MODE_PREF = "tenshi.enableDebug";
var MAIN_WINDOW_TYPE = "angel-player:main";
var debugMode = false;

debugModule.init = function() {
    debugMode = Services.prefs.getBoolPref(DEBUG_MODE_PREF);

    // Start remote debugger
    if (!DebuggerServer.initialized) {
      DebuggerServer.init();
      DebuggerServer.addBrowserActors(MAIN_WINDOW_TYPE);
    }
    DebuggerServer.openListener(6000);
};

debugModule.isDebugEnabled = function() {
    return debugMode;
};

debugModule.toggleDebug = function() {
    debugMode = !debugMode;
    Services.prefs.setBoolPref(DEBUG_MODE_PREF, debugMode);
};
