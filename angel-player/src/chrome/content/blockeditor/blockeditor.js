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

const { Cu } = require("chrome");

var $;
var svgUtil = require("tenshi/blockeditor/svgUtil");
var blocksCommon = require("tenshi/blockeditor/blockscommon");
var blockDnD = require("tenshi/blockeditor/blockDragDrop");
var blockProgram = require("tenshi/blockeditor/blockProgram");

var myProg;

var window;
var document;

exports.init = function(_window) {
    window = _window;
    document = window.document;
    $ = require("jquery")(window);
    $(function() {
        onLoad();
    });
};

function onLoad() {
    svgUtil.init(document);
    blockDnD.init(document);
    blockProgram.init(document);

    myProg = blockProgram.createNewProgram(document);
    blockDnD.setProgram(myProg);

    // test123
    var herp = blockProgram.createNewBlock(document,
        blocksCommon.BLOCK_TYPE_COMMENT, 'test123', 0, 0);
    myProg.addRootBlock(herp);

    // test456
    herp = blockProgram.createNewBlock(document,
        blocksCommon.BLOCK_TYPE_LVALUE, 'test456', 0, 5);
    myProg.addRootBlock(herp);
}
