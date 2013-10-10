Components.utils.import("chrome://angel-player/content/svgUtil.js");
Components.utils.import("chrome://angel-player/content/blockscommon.js");

function onLoad() {
    svgUtil.init(document);

    // test123
    var herp = svgUtil.createBlock('test123', blocksCommon.BLOCK_END_FLAT, blocksCommon.BLOCK_END_FLAT);
    document.getElementById('blocks-main').appendChild(herp);

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
}
