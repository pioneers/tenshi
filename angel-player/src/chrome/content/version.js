Components.utils.import("resource://gre/modules/Services.jsm");

var appVersion = (function() {
    return {
        getOS : function() {
            var os = "Unknown";
            if (navigator.platform.search(/mac/i) > -1)
                os = "Mac";
            if (navigator.platform.search(/win/i) > -1)
                os = "Windows";
            if (navigator.platform.search(/linux/i) > -1)
                os = "Linux";
            return os;
        },
        getVersion : function() {
            return Services.appinfo.version;
        },
        getBuildID : function() {
            return Services.appinfo.appBuildID;
        },
    };
}());
