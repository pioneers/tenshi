Components.utils.import("chrome://angel-player/content/svgUtil.js");
Components.utils.import("chrome://angel-player/content/blockscommon.js");

var EXPORTED_SYMBOLS = ['blockProgram'];

var MAIN_SVG_ID = 'blocks-main';

var blockProgram = {};

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

    return newProg;
};

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

    // Create block SVG
    switch (blockType) {
        case blocksCommon.BLOCK_TYPE_COMMENT:
            newBlock.svgElem = svgUtil.createBlock(text,
                blocksCommon.BLOCK_END_FLAT, blocksCommon.BLOCK_END_FLAT);
            break;
        case blocksCommon.BLOCK_TYPE_LVALUE:
            newBlock.svgElem = svgUtil.createBlock(text,
                blocksCommon.BLOCK_END_FLAT,
                blocksCommon.BLOCK_END_INNER_ARROW);
            break;
        default:
            throw "Unknown block type!";
    }

    // Set transform
    // TODO(rqou): Passing in document just for this is kinda dumb
    var transform = document.getElementById(MAIN_SVG_ID).createSVGTransform();
    transform.setTranslate(x, y);
    newBlock.svgElem.transform.baseVal.initialize(transform);

    return newBlock;
};
