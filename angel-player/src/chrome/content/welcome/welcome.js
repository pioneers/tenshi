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

const { Cc, Ci } = require('chrome');
const global_state = require('tenshi/common/global_state');
const robot_application = require('tenshi/common/robot_application');
const serport = require('tenshi/common/serport');
const pieles = require('tenshi/pieles/pieles');
const piemos_radio = require('tenshi/common/piemos_radio');
const radio = require('tenshi/common/radio');
const ioports = require('tenshi/welcome/ioports');

let window = require('tenshi/common/window')();
let {document, $} = window;

function connectRadio() {
    disconnectRadio();

    let serPortName = ioports.get_serial_port();
    if (!serPortName) {
        throw "No serial port set!";
    }

    let serportObj = serport.open(serPortName);

    global_state.set('serial_port_object', serportObj);

    let addr = ioports.get_xbee_addr();
    let main_radio = global_state.get('main_radio');
    main_radio.connectXBee(addr, serportObj);

    pieles.attachRadio('piemos', new piemos_radio.Radio(addr, serportObj));
}

function disconnectRadio() {
    let main_radio = global_state.get('main_radio');
    main_radio.disconnectXBee();
    pieles.detachRadio('piemos');
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
