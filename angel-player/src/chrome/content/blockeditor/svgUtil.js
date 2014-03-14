var blocksCommon = require("tenshi/blockeditor/blockscommon");

var SVG_NS = 'http://www.w3.org/2000/svg';
var BLOCKS_HEIGHT = 1.75;
var BLOCKS_CAP_WIDTH = 1;

var document = null;
var widthComputingGroup = null;

// This function is required so that the JSM can get access to document.
exports.init = function(document_) {
    document = document_;
    // This is used because we can't get the width of unrendered objects.
    widthComputingGroup = document.getElementById('width-computing-group');
};

// Return the width of an element by adding it to the DOM, measuring it,
// and then removing it.
exports.measureElementDimensions = function(elem) {
    widthComputingGroup.appendChild(elem);
    var bbox = elem.getBBox();
    var width = bbox.width;
    var height = bbox.height;
    widthComputingGroup.removeChild(elem);
    return {'w': width, 'h': height};
};

// Create and return an element that contains the passed in text and an
// enclosing rectangle. The rectangle exactly encloses the text in the x-axis.
// The top left is at (0,0) in this object.
exports.createTextBlockPart = function(text) {
    var textNode = document.createElementNS(SVG_NS, 'text');
    // SVG text is positioned by baseline, so we need to shift it down by 1 em.
    // The remaining offset is so center it in the bounding rectangle.
    textNode.setAttributeNS(null, 'transform',
        'translate(0,' + (1 + (BLOCKS_HEIGHT - 1) / 2) + ')');
    // Because of the hackery to get the SVG in em-units, we need to set the
    // font size to 1 user unit = 1 em
    textNode.setAttributeNS(null, 'font-size', '1');
    textNode.setAttributeNS(null, 'class', 'blockText');
    textNode.appendChild(document.createTextNode(text));
    var textLength = exports.measureElementDimensions(textNode).w;

    var rectNode = document.createElementNS(SVG_NS, 'rect');
    rectNode.setAttributeNS(null, 'width', textLength);
    rectNode.setAttributeNS(null, 'height', BLOCKS_HEIGHT);
    rectNode.setAttributeNS(null, 'class', 'blockTextBkg');

    var containerGroupNode = document.createElementNS(SVG_NS, 'svg');
    containerGroupNode.setAttributeNS(null, "class", "blockTextGroup");
    containerGroupNode.appendChild(rectNode);
    containerGroupNode.appendChild(textNode);
    return containerGroupNode;
};

function generateEndCapNode(path, flip) {
    var endCap = document.createElementNS(SVG_NS, 'svg');
    endCap.setAttributeNS(null, 'width', BLOCKS_CAP_WIDTH);
    endCap.setAttributeNS(null, 'height', BLOCKS_HEIGHT);
    endCap.setAttributeNS(null, 'viewBox',
        '0 0 ' + BLOCKS_CAP_WIDTH + ' ' + BLOCKS_HEIGHT);
    endCap.setAttributeNS(null, 'class', 'blockEndCap');
    var endCapPath = document.createElementNS(SVG_NS, 'path');
    endCapPath.setAttributeNS(null, 'd', path);
    if (flip) {
        endCapPath.setAttributeNS(null, 'transform', 'matrix(-1 0 0 1 1 0)');
    }
    endCap.appendChild(endCapPath);

    return endCap;
}

// Create a normal, single-row block with the given text and end caps.
exports.createBlock = function(text, leftCap, rightCap) {
    var mainTextPart = exports.createTextBlockPart(text);

    // TODO(rqou): make this end cap generation not hardcoded
    var facingOutCapPath = "M1 0 L0 0.875 L1 1.75 Z";
    var facingInCapPath = "M1 0 L0 0 L1 0.875 L0 1.75 L1 1.75 Z";

    var leftEndCap = null;
    var rightEndCap = null;

    // Generate left end cap
    switch (leftCap) {
        case blocksCommon.BLOCK_END_FLAT:
            // Nothing to do here
            break;
        case blocksCommon.BLOCK_END_INNER_ARROW:
            // Arrow pointing inwards
            // First requires shifting the main text element
            mainTextPart.setAttributeNS(null,
                'transform', 'translate(' + BLOCKS_CAP_WIDTH + ',0)');
            // Now we create the end cap
            leftEndCap = generateEndCapNode(facingInCapPath, false);
            break;
        case blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT:
            // Arrow pointing outwards, persistent version
            // First requires shifting the main text element
            mainTextPart.setAttributeNS(null,
                'transform', 'translate(' + BLOCKS_CAP_WIDTH + ',0)');
            // Now we create the end cap
            leftEndCap = generateEndCapNode(facingOutCapPath, false);
            break;
        // TODO(rqou): Proximity thing
    }

    var rightCapPosition = exports.measureElementDimensions(mainTextPart).w;
    if (leftCap) {
        rightCapPosition += BLOCKS_CAP_WIDTH;
    }

    // Generate right end cap
    switch (rightCap) {
        case blocksCommon.BLOCK_END_FLAT:
            // Nothing to do here
            break;
        case blocksCommon.BLOCK_END_INNER_ARROW:
            // Arrow pointing inwards
            // Now we create the end cap
            rightEndCap = generateEndCapNode(facingInCapPath, true);
            // Note: for some reason using x with non-integer values doesn't
            // seem to quite work...
            rightEndCap.setAttributeNS(null,
                'transform', 'translate(' + rightCapPosition + ',0)');
            break;
        case blocksCommon.BLOCK_END_OUTER_ARROW_PERSISTENT:
            // Arrow pointing outwards, persistent version
            // Now we create the end cap
            rightEndCap = generateEndCapNode(facingOutCapPath, true);
            rightEndCap.setAttributeNS(null,
                'transform', 'translate(' + rightCapPosition + ',0)');
            break;
        // TODO(rqou): Proximity thing
    }

    // Now we put all the pieces together
    var resultingGroup = document.createElementNS(SVG_NS, 'svg');
    resultingGroup.setAttributeNS(null, "class", "codeBlock");
    resultingGroup.appendChild(mainTextPart);
    if (leftEndCap) {
        resultingGroup.appendChild(leftEndCap);
    }
    if (rightEndCap) {
        resultingGroup.appendChild(rightEndCap);
    }

    // Get size of entire thing
    var finalDimensions = exports.measureElementDimensions(resultingGroup);

    return {'svg': resultingGroup,
            'w':   finalDimensions.w,
            'h':   finalDimensions.h};
};
