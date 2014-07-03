/* jshint globalstrict: true */
"use strict";

const { Cu } = require("chrome");
const { Services } = Cu.import("resource://gre/modules/Services.jsm");

function getOS() {
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
}

function getVersion() {
    return Services.appinfo.version;
}

function getBuildID() {
    return Services.appinfo.appBuildID;
}

exports.getOS = getOS;
exports.getVersion = getVersion;
exports.getBuildID = getBuildID;
