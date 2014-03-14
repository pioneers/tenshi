Components.utils.import("chrome://angel-player/content/version.js");
Components.utils.import("chrome://angel-player/content/debug.js");
Components.utils.import("chrome://angel-player/content/superglobals.js");

var XUL_MAIN_CONTENT_ID = 'mainXULBrowser';
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

    // Set up globals object. This globals object lives inside the main page
    // document. The main page does not normally reload, and subpages can add
    // and reference items in here. However, the "super-reload" function will
    // wipe this scope. "Super-reload" will not wipe superglobals that are
    // stored in a JSM context.
    document.tenshiGlobals = {};

    // Set up main loader object. You must import the Add-on SDK loader.js first
    // in the HTML page.
    document.tenshiGlobals.loader = loader.Loader({
        paths: {
            "": "resource://gre/modules/commonjs/",
            "tenshi": "chrome://angel-player/content/",
        },
    });
}

function toggleDebug() {
    debugModule.toggleDebug();
    setDebugFooterVisibility();
}

function mainContentGoBack() {
    var mainContent = document.getElementById(MAIN_CONTENT_ID);
    mainContent.contentWindow.history.back();
}

function mainContentGoForward() {
    var mainContent = document.getElementById(MAIN_CONTENT_ID);
    mainContent.contentWindow.history.forward();
}

// This function reloads the contents of the iframe inside the main page.
function mainContentReload() {
    var mainContent = document.getElementById(MAIN_CONTENT_ID);
    mainContent.contentWindow.location.reload(true);
}

// This function reloads the main page itself. It might be useful for any JS
// stored in the main page scope.
function entirePageReload() {
    // We invalidate some kind of JS cache thing so that when we reload we
    // re-eval all the global JS.
    var obs = Components.classes["@mozilla.org/observer-service;1"]
        .getService(Components.interfaces.nsIObserverService);
    obs.notifyObservers(null, "startupcache-invalidate", null);

    var browserElem =
        superGlobals.xulMainWindow.document.getElementById(XUL_MAIN_CONTENT_ID);
    browserElem.goHome();
}
