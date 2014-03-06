// Components.utils.import("chrome://angel-player/content/svgUtil.js");
Components.utils.import("chrome://angel-player/content/blockeditor/assets/javascripts/blockscommon.js");
Components.utils.import("chrome://angel-player/content/blockeditor/assets/javascripts/blockDragDrop.js");
Components.utils.import("chrome://angel-player/content/blockeditor/assets/javascripts/load.js");
// Components.utils.import("chrome://angel-player/content/blockeditor/libraries/snapsvg/dist/snap.svg.js");
// Components.utils.import("chrome://angel-player/content/blockeditor/libraries/jquery-2.0.3.js");
// Components.utils.import("chrome://angel-player/content/blockeditor/libraries/snap-plugins.js");
// Components.utils.import("chrome://angel-player/content/blockeditor/libraries/snap-plugins-jquery.js");
// Components.utils.import("chrome://angel-player/content/blockProgram.js");

var myProg;

function onLoad() {
    blockDnD.init(document);
    blockProgram.init(document);

    myProg = blockProgram.createNewProgram(document);
    blockDnD.setProgram(myProg);

    test = blockProgram.loadBlock(document, blocksCommon.BLOCK_TYPE_LVALUE, 'base_inputbody', 0, 0);
    myProg.addRootBlock(test);

    test2 = blockProgram.loadBlock(document, blocksCommon.BLOCK_TYPE_LVALUE, 'return_witharg', 100, 100);
    myProg.addRootBlock(test2);

    // // test123
    // var herp = blockProgram.createNewBlock(document,
    //     blocksCommon.BLOCK_TYPE_COMMENT, 'test123', 0, 0);
    // myProg.addRootBlock(herp);

    // // test456
    // herp = blockProgram.createNewBlock(document,
    //     blocksCommon.BLOCK_TYPE_LVALUE, 'test456', 0, 5);
    // myProg.addRootBlock(herp);

    /*
    // <test123
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT, blocksCommon.BLOCK_END_FLAT);
    herp.setAttributeNS(null, 'transform', 'translate(0,5)');
    document.getElementById('blocks-main').appendChild(herp);

    // >test123
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_INNER_ARROW, blocksCommon.BLOCK_END_FLAT);
    herp.setAttributeNS(null, 'transform', 'translate(0,10)');
    document.getElementById('blocks-main').appendChild(herp);

    // test123>
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_FLAT, blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT);
    herp.setAttributeNS(null, 'transform', 'translate(0,15)');
    document.getElementById('blocks-main').appendChild(herp);

    // test123<
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_FLAT, blocksCommon.BLOCK_END_INNER_ARROW);
    herp.setAttributeNS(null, 'transform', 'translate(0,20)');
    document.getElementById('blocks-main').appendChild(herp);

    // <test123<
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT, blocksCommon.BLOCK_END_INNER_ARROW);
    herp.setAttributeNS(null, 'transform', 'translate(0,25)');
    document.getElementById('blocks-main').appendChild(herp);

    // >test123<
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_INNER_ARROW, blocksCommon.BLOCK_END_INNER_ARROW);
    herp.setAttributeNS(null, 'transform', 'translate(0,30)');
    document.getElementById('blocks-main').appendChild(herp);

    // <test123>
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT, blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT);
    herp.setAttributeNS(null, 'transform', 'translate(0,35)');
    document.getElementById('blocks-main').appendChild(herp);

    // >test123>
    herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_INNER_ARROW, blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT);
    herp.setAttributeNS(null, 'transform', 'translate(0,40)');
    document.getElementById('blocks-main').appendChild(herp);
    */
}