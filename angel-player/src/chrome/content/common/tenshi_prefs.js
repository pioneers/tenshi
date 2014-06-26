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

// This module contains various convenience functions for global preferences.
// Currently, this includes:
//  * Whether debug is enabled

const { Cu } = require("chrome");
const { Services } = Cu.import("resource://gre/modules/Services.jsm");

const DEBUG_MODE_PREF = "tenshi.enableDebug";

function isDebugEnabled() {
    return Services.prefs.getBoolPref(DEBUG_MODE_PREF);
}

// Returns the new state of the debug flag.
function toggleDebug() {
  let debugMode = isDebugEnabled();
  debugMode = !debugMode;
  Services.prefs.setBoolPref(DEBUG_MODE_PREF, debugMode);

  return debugMode;
}

exports.isDebugEnabled = isDebugEnabled;
exports.toggleDebug = toggleDebug;
