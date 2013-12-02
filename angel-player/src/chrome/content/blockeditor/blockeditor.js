/* jshint globalstrict: true */
"use strict";

const { Cu } = require("chrome");

var svgUtil = require("tenshi/blockeditor/svgUtil");
var blocksCommon = require("tenshi/blockeditor/blockscommon");
var blockDnD = require("tenshi/blockeditor/blockDragDrop");
var blockProgram = require("tenshi/blockeditor/blockProgram");

var myProg;

exports.onLoad = function(document) {
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

    // testContainer
    herp = blockProgram.createNewBlock(document,
        blocksCommon.BLOCK_TYPE_COMMENT, 'testContainer', 0, 10);
    herp.isContainer = true;
    myProg.addRootBlock(herp);
};
