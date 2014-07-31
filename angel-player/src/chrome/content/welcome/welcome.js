/* jshint globalstrict: true */
"use strict";

const { Cc, Ci } = require('chrome');
const global_state = require('tenshi/common/global_state');
const robot_application = require('tenshi/common/robot_application');
const serport = require('tenshi/common/serport');
const pieles = require('tenshi/pieles/pieles');
const piemos_radio = require('tenshi/common/piemos_radio');
const radio = require('tenshi/common/radio');

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
    let main_radio = new radio.Radio(addr, serportObj);
    global_state.set('main_radio', main_radio);

    pieles.attachRadio('piemos', new piemos_radio.Radio(addr, serportObj));
    pieles.attachRadio('ndl3', main_radio);
}

function disconnectRadio() {
    pieles.detachRadio('piemos');
    pieles.detachRadio('ndl3');
    let serportObj = global_state.get('serial_port_object');
    if (serportObj) {
        serportObj.close();
        global_state.set('serial_port_object', undefined);
    }
}

function onLoad() {
    // TODO(rqou): For some reason there is a race condition with this and it
    // doesn't always work.
    robot_application.CreateEmptyRobotApplication();

    $("#openButton").click(onOpenClicked);
    $("#saveButton").click(onSaveClicked);
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

        robot_application.LoadRobotApplication(fp.file.path);
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

        robot_application.SaveRobotApplication(fp.file.path);
    }

    fp.open({done: saveCallback});
}

$(onLoad);
