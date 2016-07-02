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

let window;
let document;
let $;

let currentSelectedTab = -1;

const { isDebugEnabled } = require('tenshi/common/tenshi_prefs');

// Different "environments" such as coding, driving, home page, etc.
var ENVIRONMENTS = [
    {
        text: "Home",
        activate: function(prev) {
            $("#welcome-div").show();
        },
        deactivate: function(next) {
            $("#welcome-div").hide();
        },
    }, {
        text: "Code",
        image: 'chrome://angel-player/content/main-ui/assets/code.png',
        activate: function(prev) {
            $("#controls-div").show();
            $("#texteditor-div").show();
            $("#console-div").show();
            $("#apihelp-div").show();
        },
        deactivate: function(next) {
            $("#controls-div").hide();
            $("#texteditor-div").hide();
            $("#console-div").hide();
            $("#apihelp-div").hide();
        },
    }, {
        text: "Simulator",
        image: 'chrome://angel-player/content/main-ui/assets/simulator.png',
        activate: function(prev) {
            $("#controls-div").show();
            $("#texteditor-div").show();
            $("#simulator-div").show();
            $("#console-div").show();
            // This is a hack to make the simulator show up or else it makes
            // the simulator 0x0
            $(window).resize();
        },
        deactivate: function(next) {
            $("#controls-div").hide();
            $("#texteditor-div").hide();
            $("#simulator-div").hide();
            $("#console-div").hide();
        },
    }, {
        text: "Robot Builder",
        image: 'chrome://angel-player/content/main-ui/assets/builder.png',
        activate: function(prev) {
            $("#maker-div").show();
            // This is a hack to make the robot builder show up or else it makes
            // the robot builder 0x0
            $(window).resize();
        },
        deactivate: function(next) {
            $("#maker-div").hide();
        },
    },
];

// Special page for debug mode only.
// Useful for prototyping components on their own page, since this can't be done
// in a browser
// TODO(rqou): Toggling doesn't update the menu; you need to reload.
if (isDebugEnabled()) {
  ENVIRONMENTS.push({
        text: "Stargate Network",
        activate: function(prev) {
            $("#stargate-div").show();
        },
        deactivate: function(next) {
            $("#stargate-div").hide();
        },
  });
}

function newTabSelected() {
    /* jshint validthis: true */

    let prevSelectedTab = currentSelectedTab;
    let newSelectedTab = $(this).data().index;

    // Adjust the header bar
    $("#appTitle").text("AP").removeClass("emphasize");
    $("#currentEnvironment").text(ENVIRONMENTS[newSelectedTab].text).addClass("emphasize");

    // Adjust the tabs bar
    $("a.tabActive").removeClass('tabActive');
    $(this).addClass('tabActive');

    // Activate/deactivate the environments
    if (prevSelectedTab != -1) {
        ENVIRONMENTS[prevSelectedTab].deactivate(newSelectedTab);
    }

    ENVIRONMENTS[newSelectedTab].activate(prevSelectedTab);

    // Set data structures
    currentSelectedTab = newSelectedTab;
}

// Creates the tabs for the different environments
function createTabs() {
  let tabsDiv = $("#tabsDiv");
  for (let i = 0; i < ENVIRONMENTS.length; i++) {
	let environment = ENVIRONMENTS[i];
	let link = $("<a href='#'>" + environment.text + "</a>");
	link.data({"index": i});
	link.on('click', newTabSelected);
	tabsDiv.append(link);
  }
}

function onLoad() {
    $("#appTitle").addClass("emphasize");
    createTabs();
}

exports.init = function(_window) {
    window = _window;
    document = window.document;
    $ = require('jquery')(window);
    $(onLoad);
};
