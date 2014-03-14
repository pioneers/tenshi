/* jshint globalstrict: true */
"use strict";

const { Cu } = require("chrome");

// The following lines confuse the heck out of jshint. They will be removed
// in the immediate next commit when we fully move to require.
/* global svgUtil:true */
/* global blocksCommon:true */
/* global blockDnD:true */
/* global blockProgram:true */
Cu.import("chrome://angel-player/content/blockeditor/svgUtil.js");
Cu.import("chrome://angel-player/content/blockeditor/blockscommon.js");
Cu.import("chrome://angel-player/content/blockeditor/blockDragDrop.js");
Cu.import("chrome://angel-player/content/blockeditor/blockProgram.js");

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
};
