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

/* jshint globalstrict: true */
"use strict";

const { Cu } = require("chrome");
const { Services } = Cu.import("resource://gre/modules/Services.jsm");

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

exports.getOS = getOS;
exports.getVersion = getVersion;
exports.getBuildID = getBuildID;
