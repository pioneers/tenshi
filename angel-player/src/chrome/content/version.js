Components.utils.import("resource://gre/modules/Services.jsm");

var EXPORTED_SYMBOLS = ["appVersion"];

var appVersion = {};

appVersion.getOS = function() {
    var os = "Unknown";
    // TODO(rqou): Is this getting the right window?
    var window = Services.wm.getMostRecentWindow(null);
    if (window.navigator.platform.search(/mac/i) > -1)
        os = "Mac";
    if (window.navigator.platform.search(/win/i) > -1)
        os = "Windows";
    if (window.navigator.platform.search(/linux/i) > -1)
        os = "Linux";
    return os;
};

appVersion.getVersion = function() {
    return Services.appinfo.version;
};

appVersion.getBuildID = function() {
    return Services.appinfo.appBuildID;
};
