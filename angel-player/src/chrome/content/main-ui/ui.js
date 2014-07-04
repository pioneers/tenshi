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
        },
        deactivate: function(next) {
            $("#controls-div").hide();
            $("#texteditor-div").hide();
            $("#console-div").hide();
        },
    }, {
        text: "Block Editor",
        image: 'chrome://angel-player/content/main-ui/assets/blockeditor.png',
        activate: function(prev) {
            $("#controls-div").show();
            $("#blockeditor-div").show();
            $("#console-div").show();
        },
        deactivate: function(next) {
            $("#controls-div").hide();
            $("#blockeditor-div").hide();
            $("#console-div").hide();
        },
    }, {
        text: "Simulator",
        image: 'chrome://angel-player/content/main-ui/assets/simulator.png',
        activate: function(prev) {
            $("#controls-div").show();
            $("#texteditor-div").show();
            $("#simulator-div").show();
            $("#console-div").show();
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
