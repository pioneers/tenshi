Components.utils.import("chrome://angel-player/content/version.js");
Components.utils.import("chrome://angel-player/content/debug.js");

var MAIN_CONTENT_ID = 'mainContent';
var FOOTER_CONTAINER_ID = 'footer-debug-container';
var VERSION_DATA_ID = 'version-data';

function setDebugFooterVisibility() {
    var debugFooter = document.getElementById(FOOTER_CONTAINER_ID);
    if (!debugModule.isDebugEnabled()) {
        debugFooter.style.display = "none";
    }
    else {
        debugFooter.style.display = "";
    }
}

function setVersionInfoTag() {
    var infoTag = document.getElementById(VERSION_DATA_ID);
    infoTag.innerHTML = "Build ID " + appVersion.getBuildID() +
        ", Version " + appVersion.getVersion() +
        " (running on " + appVersion.getOS() + ")";
}

function onLoad() {
    setVersionInfoTag();

    // Hide debug stuff if debugging is off.
    setDebugFooterVisibility();
}

function toggleDebug() {
    debugModule.toggleDebug();
    setDebugFooterVisibility();
}

function openInspector() {
    window.open("chrome://inspector/content/inspector.xul", "", "chrome");
}

function mainContentGoBack() {
    var mainContent = document.getElementById(MAIN_CONTENT_ID);
    mainContent.contentWindow.history.back();
}

function mainContentGoForward() {
    var mainContent = document.getElementById(MAIN_CONTENT_ID);
    mainContent.contentWindow.history.forward();
}

function mainContentReload() {
    var mainContent = document.getElementById(MAIN_CONTENT_ID);
    mainContent.contentWindow.history.go(0);
}
