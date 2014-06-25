/* jshint globalstrict: true */
/* global document */
/* global window */
/* global loader */
"use strict";

function onLoad() {
    // Set up globals object. This globals object lives inside the main page
    // document. The main page does not normally reload, and subpages can add
    // and reference items in here. However, the "super-reload" function will
    // wipe this scope. "Super-reload" will not wipe superglobals that are
    // stored in a JSM context.
    document.tenshiGlobals = {};

    // Set up main loader object. You must import the Add-on SDK loader.js first
    // in the HTML page.
    document.tenshiGlobals.loader = loader.Loader({
        globals: {
            // TODO(rqou): This is a tentative hack to make referencing Angelic
            // work.
            setTimeout: window.setTimeout,
            clearTimeout: window.clearTimeout,
            // TODO(rqou): This is a hack because for some reason requiring the
            // loader within the CommonJS environment doesn't actually work.
            // This might be fixed in a later version of the Jetpack SDK.
            jetpackLoader: loader,
        },
        paths: {
            // In order to make node.js and other modules happy, the "root"
            // of the import paths is the vendor js directory. To get the
            // Mozilla Jetpack SDK stuff, use jetpack/
            "": "chrome://angel-player/content/vendor-js/",
            "jetpack": "resource://gre/modules/commonjs",
            "tenshi": "chrome://angel-player/content",
        },
    });

    var requirer = loader.Module("TEST_MAIN", "about:blank");
    var require = loader.Require(document.tenshiGlobals.loader, requirer);
    require('tenshi/test/main-commonjs').init(window);
}
