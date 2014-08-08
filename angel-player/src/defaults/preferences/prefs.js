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

pref("toolkit.defaultChromeURI", "chrome://angel-player/content/main.xul");

pref("tenshi.enableDebug", false);

// This prevents the middle mouse button from messing up things (we do our own
// scroll handling).
// TODO(rqou): Which of these are strictly necessary?
pref("middlemouse.contentLoadURL", false);
pref("middlemouse.openNewWindow", false);
pref("middlemouse.paste", false);
pref("middlemouse.scrollbarPosition", false);
pref("general.autoScroll", false);

pref("dom.gamepad.enabled", true);
// This allows us to use events to handle buttons/axes, rather than polling
// for them. This is Mozilla-only!
pref("dom.gamepad.non_standard_events.enabled", true);
