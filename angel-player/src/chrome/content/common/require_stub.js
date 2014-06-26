// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

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

    var requirer = loader.Module("HTML_UI", document.URL);

    return loader.Require(globalLoader, requirer);
})();


function requireStub(moduleName) {
    var module = require(moduleName);
    if (module.init !== undefined) {
        module.init(window);
    }
    return module;
}
