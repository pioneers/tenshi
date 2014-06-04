/* jshint globalstrict: true */
"use strict";

let window;
let document;
let $;

let currentSelectedTab = -1;

const { Cu } = require("chrome");
let debugModule =
    Cu.import("chrome://angel-player/content/debug.js").debugModule;

// Different ways to arrange the divs to suit different environments. Contains
// relative flexbox sizes for each element.
const LAYOUTS = {
    "center-with-all-sidebars": {
        top: "1",
        centerGroup: "8",
        bottom: "1",
        left: "1",
        centerPane: "8",
        right: "1",
    },
    "center-only": {
        top: "0",
        centerGroup: "1",
        bottom: "0",
        left: "0",
        centerPane: "1",
        right: "0",
    },
    "split-view-header-footer": {
        top: "1",
        centerGroup: "7",
        bottom: "2",
        left: "1",
        centerPane: "0",
        right: "1",
    }
};

// Different "environments" such as coding, driving, home page, etc.
var ENVIRONMENTS = [
    {
        text: "Home",
        image: 'chrome://angel-player/content/main-ui/assets/home.png',
        activate: function(prev) {
            switchLayout("center-only");
            loadPageIntoDiv("centerDiv", "../welcome/welcome.html");
        },
        deactivate: function(next) {
            $("#centerDiv").empty();
        },
    }, {
        text: "Code",
        image: 'chrome://angel-player/content/main-ui/assets/code.png',
        activate: function(prev) {
            switchLayout("split-view-header-footer");
            loadPageIntoDiv("topDiv", "../controls/main.html");
            loadPageIntoDiv("leftDiv", "../texteditor/editor.html");
            // TODO(rqou): API help page
            loadPageIntoDiv("bottomDiv", "../console/console.html");
        },
        deactivate: function(next) {
            $("#topDiv").empty();
            $("#leftDiv").empty();
            $("#bottomDiv").empty();
        },
    }, {
        text: "Block Editor",
        image: 'chrome://angel-player/content/main-ui/assets/blockeditor.png',
        activate: function(prev) {
            switchLayout("split-view-header-footer");
            loadPageIntoDiv("topDiv", "../controls/main.html");
            loadPageIntoDiv("leftDiv", "../blockeditor/blockeditor.html");
            // TODO(rqou): API help page
            loadPageIntoDiv("bottomDiv", "../console/console.html");
        },
        deactivate: function(next) {
            $("#topDiv").empty();
            $("#leftDiv").empty();
            $("#bottomDiv").empty();
        },
    }, {
        text: "Simulator",
        image: 'chrome://angel-player/content/main-ui/assets/simulator.png',
        activate: function(prev) {
            // TODO(rqou): Don't unload a page just to reload it again!
            // TODO(rqou): A way to map names back to indices.
            switchLayout("split-view-header-footer");
            loadPageIntoDiv("topDiv", "../controls/main.html");
            if (prev == 2) {
                // Block editor was previous
                loadPageIntoDiv("leftDiv", "../blockeditor/blockeditor.html");
            } else {
                loadPageIntoDiv("leftDiv", "../texteditor/editor.html");
            }
            loadPageIntoDiv("rightDiv", "../simulator/main.html");
            loadPageIntoDiv("bottomDiv", "../console/console.html");
        },
        deactivate: function(next) {
            $("#topDiv").empty();
            $("#leftDiv").empty();
            $("#rightDiv").empty();
            $("#bottomDiv").empty();
        },
    }, {
        text: "Robot Builder",
        image: 'chrome://angel-player/content/main-ui/assets/builder.png',
        activate: function(prev) {
            switchLayout("center-only");
            loadPageIntoDiv("centerDiv", "../simulator/maker.html");
        },
        deactivate: function(next) {
            $("#centerDiv").empty();
        },
    },
];

// Special page for debug mode only.
// Useful for prototyping components on their own page, since this can't be done
// in a browser
// TODO(rqou): Toggling doesn't update the menu; you need to reload.
if (debugModule.isDebugEnabled()) {
  ENVIRONMENTS.push({
        text: "Stargate Network",
        image: 'chrome://angel-player/content/main-ui/assets/stargate.png',
        activate: function(prev) {
            switchLayout("center-only");
            loadPageIntoDiv("centerDiv", "../debug-main.html");
        },
        deactivate: function(next) {
            $("#centerDiv").empty();
        },
  });
}

function loadPageIntoDiv(div, page) {
    // TODO(rqou): Refactor paths
    $("<iframe class=\"absolutelyNoSpace\" " +
        "src=\"" + page + "\"></iframe>").appendTo($("#" + div));
}

function switchLayout(newLayout) {
    let layout = LAYOUTS[newLayout];
    $("#topDiv").css('flex', layout.top);
    $("#mainCenter").css('flex', layout.centerGroup);
    $("#bottomDiv").css('flex', layout.bottom);
    $("#leftDiv").css('flex', layout.left);
    $("#centerDiv").css('flex', layout.centerPane);
    $("#rightDiv").css('flex', layout.right);
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
