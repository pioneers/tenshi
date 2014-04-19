/* jshint globalstrict: true */
"use strict";

const { Cc, Ci } = require('chrome');
const global_state = require('tenshi/common/global_state');
const robot_application = require('tenshi/common/robot_application');
let window;
let document;
let $;

function xbeeAddrKeyup(e) {
    /* jshint validthis: true */

    if (this.validity.valid) {
        global_state.get('robot_application').radio_pairing_info =
            this.value;
    }
}

function updateFromSavedRobotApp() {
    let robotApp = global_state.get('robot_application');

    if (robotApp.radio_pairing_info) {
        $("#xbeeAddrInput").val(robotApp.radio_pairing_info);
    }
}

function onLoad() {
    if (!global_state.get('robot_application')) {
        let emptyApp = robot_application.CreateEmptyRobotApplication();
        // This assumes welcome is loaded first before other pages. Seems to be
        // a fine assumption for now.
        global_state.set('robot_application', emptyApp);
    }

    updateFromSavedRobotApp();

    $("#xbeeAddrInput").keyup(xbeeAddrKeyup);
    $("#openButton").click(onOpenClicked);
    $("#saveButton").click(onSaveClicked);
}

function onOpenClicked(e) {
    let fp = Cc["@mozilla.org/filepicker;1"].createInstance(Ci.nsIFilePicker);
    fp.init(window, "Open program", Ci.nsIFilePicker.modeOpen);
    fp.defaultExtension = "yaml";
    fp.defaultString = "*.yaml";
    fp.appendFilter("YAML files", "*.yaml");

    function openCallback(result) {
        if (result == Ci.nsIFilePicker.returnCancel) {
            return;
        }

        global_state.set('robot_application',
            robot_application.LoadRobotApplication(fp.file.path));

        updateFromSavedRobotApp();
    }

    fp.open({done: openCallback});
};

function onSaveClicked(e) {
    let fp = Cc["@mozilla.org/filepicker;1"].createInstance(Ci.nsIFilePicker);
    fp.init(window, "Save program", Ci.nsIFilePicker.modeSave);
    fp.defaultExtension = "yaml";
    fp.defaultString = "untitled.yaml";
    fp.appendFilter("YAML files", "*.yaml");

    function saveCallback(result) {
        if (result == Ci.nsIFilePicker.returnCancel) {
            return;
        }

        global_state.get('robot_application').save(fp.file.path);
    }

    fp.open({done: saveCallback});
};

exports.init = function(_window) {
    window = _window;
    document = window.document;
    $ = require('jquery')(window);
    $(onLoad);
};
