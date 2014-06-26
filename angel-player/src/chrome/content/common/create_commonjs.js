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
            // Leak the global window to "common/window.js"
            'get_global_window': function() {return window;}
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
