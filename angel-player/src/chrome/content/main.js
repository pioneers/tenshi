Components.utils.import("resource://gre/modules/Services.jsm");

var XUL_MAIN_CONTENT_ID = 'mainXULBrowser';
var MAIN_CONTENT_ID = 'mainContent';
var FOOTER_CONTAINER_ID = 'footer-debug-container';
var VERSION_DATA_ID = 'version-data';
var DEBUG_MODE_PREF = "tenshi.enableDebug";
var debugEnabled;

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

function setDebugFooterVisibility(visible) {
    var debugFooter = document.getElementById(FOOTER_CONTAINER_ID);
    if (visible) {
        debugFooter.style.display = "";
    }
    else {
        debugFooter.style.display = "none";
    }
}

function setVersionInfoTag() {
    var infoTag = document.getElementById(VERSION_DATA_ID);
    infoTag.innerHTML = "Build ID " + getBuildID() +
        ", Version " + getVersion() +
        " (running on " + getOS() + ")";
}

function onLoad() {
    setVersionInfoTag();

    // Set initial visibility.
    debugEnabled = Services.prefs.getBoolPref(DEBUG_MODE_PREF);
    setDebugFooterVisibility(debugEnabled);

    // Set up globals object. This globals object lives inside the main page
    // document. The main page does not normally reload, and subpages can add
    // and reference items in here. However, the "super-reload" function will
    // wipe this scope. "Super-reload" will not wipe anything in a JSM context.
    document.tenshiGlobals = {};

    // Set up main loader object. You must import the Add-on SDK loader.js first
    // in the HTML page.
    document.tenshiGlobals.loader = loader.Loader({
        globals: {
            // TODO(rqou): This is a tentative hack to make referencing Angelic
            // work.
            setTimeout: window.setTimeout,
            clearTimeout: window.clearTimeout,
        },
        modules: {
            'toolkit/loader': loader,
        },
        paths: {
            // In order to make node.js and other modules happy, the "root"
            // of the import paths is the vendor js directory. To get the
            // Mozilla Jetpack SDK stuff, use sdk/
            "": "chrome://angel-player/content/vendor-js/",
            "sdk": "resource://gre/modules/commonjs/sdk",
            "tenshi": "chrome://angel-player/content",
        },
    });

    // TODO(rqou): This loading is inefficient. However, if we don't do this,
    // we get a race condition where ui.html tries to load tenshiGlobals before
    // we even create it.
    document.getElementById('mainContent').src = "main-ui/ui.html";
}

function toggleDebug() {
    debugEnabled = !debugEnabled;
    Services.prefs.setBoolPref(DEBUG_MODE_PREF, debugEnabled);
    setDebugFooterVisibility(debugEnabled);
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

    var xulMainWindow = window
        .QueryInterface(Components.interfaces.nsIInterfaceRequestor)
        .getInterface(Components.interfaces.nsIWebNavigation)
        .QueryInterface(Components.interfaces.nsIDocShellTreeItem)
        .rootTreeItem
        .QueryInterface(Components.interfaces.nsIInterfaceRequestor)
        .getInterface(Components.interfaces.nsIDOMWindow);
    var browserElem =
        xulMainWindow.document.getElementById(XUL_MAIN_CONTENT_ID);
    browserElem.goHome();
}
