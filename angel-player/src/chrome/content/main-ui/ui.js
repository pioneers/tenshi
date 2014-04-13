/* jshint globalstrict: true */
"use strict";

let window;
let document;
let $;

let currentSelectedTab = -1;

const XUL_XMLNS =
    'http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul';

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
    }
};

// Different "environments" such as coding, driving, home page, etc.
const ENVIRONMENTS = [
    {
        text: "Home",
        image: 'chrome://angel-player/content/main-ui/assets/home.png',
        activate: function() {
            switchLayout("center-with-all-sidebars");
        },
        deactivate: function() {},
    }, {
        text: "Code",
        image: 'chrome://angel-player/content/main-ui/assets/code.png',
        activate: function() {
            switchLayout("center-only");
        },
        deactivate: function() {},
    }
];

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

    if (currentSelectedTab != -1) {
        ENVIRONMENTS[currentSelectedTab].deactivate();
    }

    currentSelectedTab = this.selectedIndex;
    ENVIRONMENTS[currentSelectedTab].activate();
}

// Creates the (XUL) tabs for the different environments
function createTabs() {
    let tabsXULElem = document.createElementNS(XUL_XMLNS, "tabs");
    // This does not work in a HTML content document; we probably don't need
    // this functionality anyways.
    tabsXULElem.setAttributeNS(null, 'setfocus', false);

    for (let environment of ENVIRONMENTS) {
        let envTab = document.createElementNS(XUL_XMLNS, "tab");
        envTab.setAttributeNS(null, 'label', environment.text);
        if (environment.image) {
            envTab.setAttributeNS(null, 'image', environment.image);
        }
        tabsXULElem.appendChild(envTab);
    }

    tabsXULElem.addEventListener('select', newTabSelected);
    $("#tabsDiv").append(tabsXULElem);
}

function onLoad() {
    createTabs();
}

exports.init = function(_window) {
    window = _window;
    document = window.document;
    $ = require('jquery')(window);
    $(onLoad);
};
