// This file is intended to be loaded in a <script> tag in "toplevel" HTML
// documents only. It's one function, createCommonJsEnv, returns a loader
// object from the Jetpack SDK that has all of the normal paths set up for
// the angel-player project. The argument is the "loader" object from the
// Jetpack SDK. It is an argument because its name varies depending on how
// loader.js was imported.

function createCommonJsEnv(loaderModule) {
    return loaderModule.Loader({
        globals: {
            // TODO(rqou): Check to make sure that this is the "correct"
            // setTimeout/etc. to use and that no problems occur by using one
            // attached to the current window.
            setTimeout: window.setTimeout,
            clearTimeout: window.clearTimeout,
            setInterval: window.setInterval,
            clearInterval: window.clearInterval,
            console: console,   // "console" object from the Web API
        },
        modules: {
            // So that re-importing the loader works from within CommonJS
            'toolkit/loader': loaderModule,
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
}
