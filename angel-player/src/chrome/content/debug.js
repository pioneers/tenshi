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

Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
Components.utils.import("resource://gre/modules/Services.jsm");
const console =
  Components.utils.import("resource://gre/modules/devtools/Console.jsm").console;

const MAIN_WINDOW_TYPE = "angel-player:main";

function xulrunnerRemoteDebugInit() {
  // Start remote debugger (won't actually start unless
  // devtools.debugger.remote-enabled is set to true)
  if (!DebuggerServer.initialized) {
    DebuggerServer.init(function() {
      // Don't warn if debugging connections restricted to the local machine.
      return Services.prefs.getBoolPref("devtools.debugger.force-local") ||
        DebuggerServer._defaultAllowConnection();
    });
    DebuggerServer.addBrowserActors(MAIN_WINDOW_TYPE);

    DebuggerServer.addActors("chrome://angel-player/content/XULRootActor.js");
  }
  try {
    var port =
      Services.prefs.getIntPref('devtools.debugger.remote-port') || 6000;
    DebuggerServer.openListener(port);
  } catch (_) {
    console.log("Failed to launch DebuggerServer!");
  }
}
