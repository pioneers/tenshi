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

// This file exposes the window object (corresponding to ui.html) to the
// CommonJS environment, as well as any libraries defined in that window
// object.
//
// Usage:
//   const window = require('tenshi/common/window')();
//   let { document, $, ... } = window;

// Get the "global" window. This is right outside the iframe that has
// ui.html in it
const global_window = require('get_global_window')();

// Make the window inside the iframe available to code
module.exports = function() {
  return global_window.document.getElementById('mainContent').contentWindow;
};

