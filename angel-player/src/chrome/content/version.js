Components.utils.import("resource://gre/modules/Services.jsm");

var appVersion = (function() {
    var os = "Unknown";
    var version = "VERSION";
    var buildID = "BUILDID";

    return {
        init : function() {
            // Determine OS
            if (navigator.platform.search(/mac/i) > -1)
                os = "Mac";
            if (navigator.platform.search(/win/i) > -1)
                os = "Windows";
            if (navigator.platform.search(/linux/i) > -1)
                os = "Linux";

            // Get the version and build id
            version = Services.appinfo.version;
            buildID = Services.appinfo.appBuildID;
        },
        getOS : function() {
            return os;
        },
        getVersion : function() {
            return version;
        },
        getBuildID : function() {
            return buildID;
        },
    };
}());
