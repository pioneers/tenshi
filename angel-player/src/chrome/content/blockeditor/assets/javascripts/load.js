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

    this.svg.addTransform(Snap.Matrix(x, y));
}


blockProgram.loadBlock = function(document, blockType, block, x, y) {
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

    Snap.load("assets/images/" + block + ".svg", function(f) {
        main = Snap("#workarea");
        main.append(f.select("style"));
        main.append(f.select("pattern"));
        main.append(f.select("font"));
        f.select(".block").attr("class", "temp");
        main.append(f.select(".temp"));
        wrapper = main.select(".temp");
        wrapper.attr("class", "block");
        box = wrapper.getSmartBBox();
        newBlock.w = box.width;
        newBlock.h = box.height;
        wrapper.addTransform((new Snap.Matrix()).translate(x, y));
        newBlock.svgElem = wrapper.node;
        newBlock.svgElem.blockData = newBlock;
        newBlock.moveTo = blockMoveTo;


    // Add an input area to modify the text
    var emb = main.createEmbeddedHTML("50%", "10px", "50%", "30px");
    emb.html.append("<form accept-charset='utf-8'> Block Text: <input id='resizeupdatetext' value='text'></input></form>");


    // Event handler to change the text inside the block
    $("#resizeupdatetext").keyup(updateText);
    $("#resizeupdatetext").change(updateText);
    updateText();
    });

    return newBlock;
};

//make sure that the newBlock thing is updated whever it ends up...
function updateText() {
    var s = Snap("#workarea");
    var obj = s.select(".block"),
        val = $("input").val(),
        old_bbox = obj.select("text").getSmartBBox(),
        test_bbox = obj.getSmartBBox(),
        ref_x = test_bbox.cx,
        ref_y = test_bbox.cy,
        new_bbox;

    if (val.length < 1) {
      return;
    }

    obj.select("text").attr("text", val);
    new_bbox = obj.select("text").getSmartBBox();
    obj.resizeAtPoint(ref_x, ref_y,
                      new_bbox.width - old_bbox.width,
                      new_bbox.height - old_bbox.height,
                      "text");
}

blockProgram.init = function(document_) {
    document = document_;
    // Main toplevel SVG object
    mainSvg = document.getElementById(MAIN_SVG_ID);
};