/* jshint globalstrict: true */
"use strict";

const { Cc, Ci } = require('chrome');
const { isDebugEnabled, toggleDebug } = require('tenshi/common/tenshi_prefs');
const { getBuildID, getVersion, getOS } = require('tenshi/common/versioninfo');

var XUL_MAIN_CONTENT_ID = 'mainXULBrowser';
var MAIN_CONTENT_ID = 'mainContent';
var FOOTER_CONTAINER_ID = 'footer-debug-container';
var VERSION_DATA_ID = 'version-data';
var DEBUG_MODE_PREF = "tenshi.enableDebug";
var debugEnabled;

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
    setDebugFooterVisibility(isDebugEnabled());

    // TODO(rqou): This loading is inefficient. However, if we don't do this,
    // we get a race condition where ui.html tries to load tenshiGlobals before
    // we even create it.
    document.getElementById('mainContent').src = "main-ui/ui.html";
    
    $('#toggle-debug').click(toggleDebugFooter);
    $('#backButton').click(mainContentGoBack);
    $('#forwardButton').click(mainContentGoForward);
    $('#reloadButton').click(mainContentReload);
    $('#superReloadButton').click(entirePageReload);
}

function toggleDebugFooter() {
    var debugEnabled = toggleDebug();
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
    var obs = Cc["@mozilla.org/observer-service;1"]
        .getService(Ci.nsIObserverService);
    obs.notifyObservers(null, "startupcache-invalidate", null);

    var xulMainWindow = window
        .QueryInterface(Ci.nsIInterfaceRequestor)
        .getInterface(Ci.nsIWebNavigation)
        .QueryInterface(Ci.nsIDocShellTreeItem)
        .rootTreeItem
        .QueryInterface(Ci.nsIInterfaceRequestor)
        .getInterface(Ci.nsIDOMWindow);
    var browserElem =
        xulMainWindow.document.getElementById(XUL_MAIN_CONTENT_ID);
    browserElem.goHome();
}

var $, window, document;

exports.init = function(_window) {
    window = _window;
    document = window.document;
    $ = window.$;

    $(onLoad);
};
