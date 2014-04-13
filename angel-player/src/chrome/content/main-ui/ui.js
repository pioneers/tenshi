let window;
let document;
let $;

let currentSelectedTab = -1;

const XUL_XMLNS =
    'http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul';

// Different "environments" such as coding, driving, home page, etc.
const ENVIRONMENTS = [
    {
        text: "Home",
        image: 'chrome://angel-player/content/main-ui/assets/home.png',
        activate: function() {dump('asdf1\n');},
        deactivate: function() {dump('asdf2\n');},
    }, {
        text: "Code",
        image: 'chrome://angel-player/content/main-ui/assets/code.png',
        activate: function() {dump('asdf3\n');},
        deactivate: function() {dump('asdf4\n');},
    }
];

function newTabSelected() {
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
