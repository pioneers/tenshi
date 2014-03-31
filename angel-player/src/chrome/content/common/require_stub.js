// This file defines a requireStub function that is used to bootstrap a page's
// use of the Mozilla Jetpack require framework. The way this file is used is
// to first add
// <script type='application/javascript;version=1.7'
//     src='resource://gre/modules/commonjs/toolkit/loader.js'></script>
// <script src="../common/require_stub.js"></script>
// to the head section of a page. Then, inside another script section, call
// requireStub(<path>). This will load 'chrome://angel-player/content/<path>.js'
// with module id 'tenshi/<path>' and call the init() method in it, passing it
// window.

var require = (function(){
    var frame = window;
    while (!frame.document.hasOwnProperty("tenshiGlobals")) {
        if (frame === frame.parent) {
            throw "tenshiGlobals not found";
        }
        frame = frame.parent;
    }

    var globalLoader = frame.document.tenshiGlobals.loader;

    var requirer = loader.Module("HTML_UI", "about:blank");

    return loader.Require(globalLoader, requirer);
})();


function requireStub(moduleName) {
    var module = require("tenshi/" + moduleName);
    module.init(window);
    return module;
}
