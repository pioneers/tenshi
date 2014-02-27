Components.utils.import("chrome://angel-player/content/svgUtil.js");
Components.utils.import("chrome://angel-player/content/blockscommon.js");

var EXPORTED_SYMBOLS = ['blockProgram'];

var MAIN_SVG_ID = 'workarea';

var document = null;
var mainSvg = null;

var blockProgram = {};

function testAABBIntersect(x1, y1, w1, h1, x2, y2, w2, h2) {
    return !((x1 + w1) < x2 ||
             (y1 + h1) < y2 ||
             x1 > (x2 + w2) ||
             y1 > (y2 + h2));
}

function programAddRootBlock(block) {
    this.rootCodeBlocks.push(block);
    this.document.getElementById(MAIN_SVG_ID).appendChild(block.svgElem);
    // TODO(rqou): Make this a binary search, not a linear search
    var i;
    for (i = 0; i < this.heightSortedBlocks.length; i++) {
        if (block.y < this.heightSortedBlocks[i].y) {
            break;
        }
    }
    this.heightSortedBlocks.splice(i, 0, block);
}

function programGetBlocksInArea(x, y, w, h) {
    var foundBlocks = [];
    // TODO(rqou): More efficient
    var i;
    for (i = 0; i < this.heightSortedBlocks.length; i++) {
        var block = this.heightSortedBlocks[i];
        if (testAABBIntersect(x, y, w, h, block.x, block.y, block.w, block.h)) {
            foundBlocks.push(block);
        }
    }

    return foundBlocks;
}

blockProgram.createNewProgram = function(document) {
    var newProg = {};

    newProg.document = document;
    // Array of "toplevel" blocks. Includes functions as well as random crap
    // scattered around the program. Does not contain a reference to all blocks,
    // only to the root blocks. In other words, this contains the blocks
    // arranged according to the program's structure.
    newProg.rootCodeBlocks = [];
    // Array of all blocks, sorted by increasing height. This is used to
    // do snapping.
    newProg.heightSortedBlocks = [];

    // Functions
    newProg.addRootBlock = programAddRootBlock;
    newProg.getBlocksInArea = programGetBlocksInArea;

    return newProg;
};

function blockMoveTo(x, y) {
    this.x = x;
    this.y = y;

    var transform = mainSvg.createSVGTransform();
    transform.setTranslate(x, y);
    this.svgElem.transform.baseVal.initialize(transform);
}

blockProgram.createNewBlock = function(document, blockType, text, x, y) {
    var newBlock = {};

    newBlock.blockType = blockType;
    newBlock.x = x;
    newBlock.y = y;
    // Outer enclosing block
    newBlock.parent = null;
    // Blocks on the left/right (args, lvalue/rvalue, etc.)
    newBlock.leftPeer = null;
    newBlock.rightPeer = null;
    // Blocks above/below in sequence
    newBlock.prevBlock = null;
    newBlock.nextBlock = null;
    // Blocks enclosed
    newBlock.firstChild = null;

    var blockSVGData;

    // Create block SVG
    switch (blockType) {
        case blocksCommon.BLOCK_TYPE_COMMENT:
            blockSVGData = svgUtil.createBlock(text,
                blocksCommon.BLOCK_END_FLAT, blocksCommon.BLOCK_END_FLAT);
            newBlock.svgElem = blockSVGData.svg;
            newBlock.w = blockSVGData.w;
            newBlock.h = blockSVGData.h;
            break;
        case blocksCommon.BLOCK_TYPE_LVALUE:
            blockSVGData = svgUtil.createBlock(text,
                blocksCommon.BLOCK_END_FLAT,
                blocksCommon.BLOCK_END_INNER_ARROW);
            newBlock.svgElem = blockSVGData.svg;
            newBlock.w = blockSVGData.w;
            newBlock.h = blockSVGData.h;
            break;
        default:
            throw "Unknown block type!";
    }

    // Set transform
    // TODO(rqou): Passing in document just for this is kinda dumb
    var transform = document.getElementById(MAIN_SVG_ID).createSVGTransform();
    transform.setTranslate(x, y);
    newBlock.svgElem.transform.baseVal.initialize(transform);

    newBlock.svgElem.blockData = newBlock;

    // Functions
    newBlock.moveTo = blockMoveTo;

    return newBlock;
};

blockProgram.init = function(document_) {
    document = document_;
    // Main toplevel SVG object
    mainSvg = document.getElementById(MAIN_SVG_ID);
};
