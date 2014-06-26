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

const file = require('sdk/io/file');
const js_yaml = require('js-yaml/lib/js-yaml');
const texteditor = require('tenshi/texteditor/editor');
const ioports = require('tenshi/welcome/ioports');
const global_state = require('tenshi/common/global_state');

// Save an application to a file
exports.SaveRobotApplication = function(filename) {
    let savedObj = {
        'text_code':            texteditor.get_text(),
        'radio_pairing_info':   ioports.get_xbee_addr(),
        'PiELES_config':        global_state.get('PiELES_config'),
    };
    
    let appstr = js_yaml.safeDump(savedObj, {skipInvalid: true});
    let outF = file.open(filename, 'w');
    // TODO(rqou): Async?
    outF.write(appstr);
    outF.close();
};

// Creates an empty robot application
exports.CreateEmptyRobotApplication = function() {
    // TODO(rqou): Remove this
    texteditor.set_text("main = fn:\n" +
                        "    while 0!=1:\n" +
                        "        x = get_sensor(1)\n" +
                        "        y = get_sensor(3)\n" +
                        "        set_motor(0, x)\n" +
                        "        set_motor(1, y)\n");
    ioports.set_xbee_addr("0013A20040A580C4");
};

// Load an application from a file
exports.LoadRobotApplication = function(filename) {
    let appstr = file.read(filename, 'r');
    let yamlObj = js_yaml.safeLoad(appstr);

    texteditor.set_text(yamlObj.text_code);
    ioports.set_xbee_addr(yamlObj.radio_pairing_info);
    global_state.set('PiELES_config', yamlObj.PiELES_config);
};
