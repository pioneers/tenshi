Components.utils.import("resource://gre/modules/Services.jsm");

var debugModule = (function() {
    var DEBUG_MODE_PREF = "tenshi.enableDebug";
    var debugMode = false;

    return {
        init : function() {
            debugMode = Services.prefs.getBoolPref(DEBUG_MODE_PREF);
        },
        isDebugEnabled : function() {
            return debugMode;
        },
        toggleDebug : function() {
            debugMode = !debugMode;
            Services.prefs.setBoolPref(DEBUG_MODE_PREF, debugMode);
        },
    }
}());
