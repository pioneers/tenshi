// This file defines require and requireStub functions that are used to
// bootstrap a page's use Mozilla Jetpack require framework. The way this file
// is used is to first add
// <script type='application/javascript;version=1.7'
//     src='resource://gre/modules/commonjs/toolkit/loader.js'></script>
// <script src="../common/require_stub.js"></script>
// to the head section of a page. This will make the require and requireStub
// functions available for later use. Given an argument 'tenshi/<path>', they
// will load 'chrome://angel-player/content/<path>.js' with module id
// 'tenshi/<path>'.
// The requireStub function will additionally call the init() method on it,
// passing the window as an argument.

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
    var module = require(moduleName);
    module.init(window);
    return module;
}
