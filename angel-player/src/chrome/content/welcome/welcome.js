/* jshint globalstrict: true */
"use strict";

const { Cc, Ci, ChromeWorker } = require('chrome');
const global_state = require('tenshi/common/global_state');
const robot_application = require('tenshi/common/robot_application');
const serport = require('tenshi/common/serport');
const pieles = require('tenshi/pieles/pieles');
const radio = require('tenshi/common/piemos_radio');

let window = require('tenshi/common/window')();
let {document, $} = window;

function connectRadio() {
    disconnectRadio();

    let serPortName = global_state.get('serial_port');
    if (!serPortName) {
        throw "No serial port set!";
    }

    let serportObj = serport.open(serPortName);

    global_state.set('serial_port_object', serportObj);

    let addr = global_state.get('robot_application').radio_pairing_info;

    pieles.attachRadio('xbee', new radio.Radio(addr, serportObj));
}

function disconnectRadio() {
    pieles.detachRadio('xbee');
    let serportObj = global_state.get('serial_port_object');
    if (serportObj) {
        serportObj.close();
        global_state.set('serial_port_object', undefined);
    }
}

function xbeeAddrKeyup() {
    /* jshint validthis: true */

    if (this.validity.valid) {
        global_state.get('robot_application').radio_pairing_info =
            this.value;
        connectRadio();
    }
}

function serialPortKeyup() {
    /* jshint validthis: true */

    if (this.validity.valid) {
        global_state.set('serial_port', this.value);
        // TODO(rqou): Persist this somewhere?
    }
}

function updateFromSavedState() {
    let robotApp = global_state.get('robot_application');

    if (robotApp.radio_pairing_info) {
        $("#xbeeAddrInput").val(robotApp.radio_pairing_info);
    }

    let serportName = global_state.get('serial_port');
    if (serportName) {
        $("#serialPort").val(serportName);
    }
}

function onLoad() {
    if (!global_state.get('robot_application')) {
        let emptyApp = robot_application.CreateEmptyRobotApplication();
        // This assumes welcome is loaded first before other pages. Seems to be
        // a fine assumption for now.
        global_state.set('robot_application', emptyApp);
    }

    updateFromSavedState();

    $("#xbeeAddrInput").keyup(xbeeAddrKeyup);
    $("#openButton").click(onOpenClicked);
    $("#saveButton").click(onSaveClicked);
    $("#serialPort").keyup(serialPortKeyup);
    $("#connectButton").click(connectRadio);
    $("#disconnectButton").click(disconnectRadio);
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

        updateFromSavedState();
    }

    fp.open({done: openCallback});
}

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
}

$(onLoad);
