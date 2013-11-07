var EXPORTED_SYMBOLS = ['blockDnD'];

var blockDnD = {};

var MOUSE_MODE_NONE = 0;
var MOUSE_MODE_PANNING = 1;
var MOUSE_MODE_DRAG_BLOCK = 2;

var document = null;
var mainSvg = null;
var mainProgram = null;
var currentMouseMode = 0;

// Current SVG panning offset. This is in pixels.
var svgPanX = 0;
var svgPanY = 0;

// These coordinates are used to track where we have started the drag, relative
// to the element being dragged. This is used for both individual elements and
// the main canvas.
var curDragX = 0;
var curDragY = 0;
var curDragElement = null;

// These are used to calculate relative dragging direction.
var oldDragX = 0;
var oldDragY = 0;

// This is used to update the sorted block array
// TODO(rqou): This is super jank because it breaks abstraction barriers
var oldBlockIdx = -1;

function dragMouseDown(evt) {
    // dump(evt);
    if (!currentMouseMode) {
        // Don't handle a new action if we're already doing one.

        switch (evt.button) {
            case 0:
                // We may at some point want to terminate some/all in-progress
                // interactions at this point.

                // Walk up the DOM until we either find either a code block
                // element or the main SVG
                var targetElem = evt.target;
                while (
                    targetElem.getAttributeNS(null, 'class') !== 'codeBlock' &&
                    targetElem !== mainSvg) {
                    targetElem = targetElem.parentNode;
                }
                // TODO(rqou): Can we ever end up with other random node?

                if (targetElem != mainSvg) {
                    // TODO(rqou): Sigh, figure out this px/em bullshit later.
                    // This is currently in px and being coverted to em
                    var clickX = (evt.clientX - svgPanX) / 16.0;
                    var clickY = (evt.clientY - svgPanY) / 16.0;

                    // Where we clicked in the block
                    curDragX = clickX - targetElem.blockData.x;
                    curDragY = clickY - targetElem.blockData.y;

                    // Where the block currently is
                    oldDragX = targetElem.blockData.x;
                    oldDragY = targetElem.blockData.y;

                    // TODO(rqou): Can I haz abstraction?
                    // Find our block
                    // TODO(rqou): Make this a binary search, not a linear search
                    var i;
                    for (i = 0; i < mainProgram.heightSortedBlocks.length;
                        i++) {
                        if (mainProgram.heightSortedBlocks[i] ===
                            targetElem.blockData) {
                            break;
                        }
                    }
                    oldBlockIdx = i;
                    dump("block is index " + i + "\n");

                    curDragElement = targetElem;
                    // TODO(rqou): Do we need to copypasta this?
                    targetElem.setAttributeNS(null, 'pointer-events', 'none');
                    currentMouseMode = MOUSE_MODE_DRAG_BLOCK;
                }

                break;
            case 1:
                // Middle mouse button, start panning
                currentMouseMode = MOUSE_MODE_PANNING;
                curDragX = evt.clientX - svgPanX;
                curDragY = evt.clientY - svgPanY;
                break;
            default:
                break;
        }
    }

    evt.preventDefault();
    evt.stopPropagation();
}

function dragMouseUp(evt) {
    switch (evt.button) {
        case 0:
            if (currentMouseMode == MOUSE_MODE_DRAG_BLOCK) {
                // Release left button, stop dragging
                currentMouseMode = MOUSE_MODE_NONE;
                curDragElement.setAttributeNS(null, 'pointer-events', 'auto');
                curDragElement = null;
            }
            break;
        case 1:
            if (currentMouseMode == MOUSE_MODE_PANNING) {
                // Release middle button, stop panning
                currentMouseMode = MOUSE_MODE_NONE;
            }
            break;
        default:
            break;
    }

    evt.preventDefault();
    evt.stopPropagation();
}

function dragMouseMove(evt) {
    // DEBUG DEBUG DEBUG

    if (evt.shiftKey) {
        var mouseX = (evt.clientX - svgPanX) / 16.0;
        var mouseY = (evt.clientY - svgPanY) / 16.0;

        dump("Mouse at " + mouseX + " " + mouseY + "\n");

        var things = mainProgram.getBlocksInArea(mouseX - 0.5, mouseY - 0.5, 1, 1);
        dump("There are " + things.length + " things\n");

        for (var i = 0; i < things.length; i++) {
            var aa = things[i];
            // dump(aa);
            dump("[" + aa.x + " " + aa.y + " " + aa.w + " " + aa.h + "]\n");
        }
    }

    // DEBUG DEBUG DEBUG

    var transform;
    switch (currentMouseMode) {
        case MOUSE_MODE_PANNING:
            // Panning the whole canvas
            svgPanX = evt.clientX - curDragX;
            svgPanY = evt.clientY - curDragY;
            transform = mainSvg.createSVGTransform();
            transform.setTranslate(svgPanX, svgPanY);
            mainSvg.transform.baseVal.initialize(transform);
            break;
        case MOUSE_MODE_DRAG_BLOCK:
            // Dragging a block

            // Figure out where we are now
            var clickX = (evt.clientX - svgPanX) / 16.0;
            var clickY = (evt.clientY - svgPanY) / 16.0;

            // New location
            var newX = clickX - curDragX;
            var newY = clickY - curDragY;

            curDragElement.blockData.x = newX;
            curDragElement.blockData.y = newY;

            transform = mainSvg.createSVGTransform();
            transform.setTranslate(newX, newY);
            curDragElement.transform.baseVal.initialize(transform);

            // Lets us figure out which way we dragged
            var deltaX = curDragElement.blockData.x - oldDragX;
            var deltaY = curDragElement.blockData.y - oldDragY;
            oldDragX = curDragElement.blockData.x;
            oldDragY = curDragElement.blockData.y;

            // Maintain the sorted block list
            if (deltaY < 0) {
                // Moving up
                if (oldBlockIdx !== 0) {
                    // Not already at the top
                    if (curDragElement.blockData.y <
                        mainProgram.heightSortedBlocks[oldBlockIdx - 1].y) {
                        // Moved past an element
                        mainProgram.heightSortedBlocks.splice(oldBlockIdx - 1,
                            2, curDragElement.blockData,
                            mainProgram.heightSortedBlocks[oldBlockIdx - 1]);
                        oldBlockIdx--;
                        dump("New index is " + oldBlockIdx + "\n");
                    }
                }
            }
            else {
                // Moving down
                if (oldBlockIdx !== mainProgram.heightSortedBlocks.length - 1) {
                    // Not already at the bottom
                    if (curDragElement.blockData.y >
                        mainProgram.heightSortedBlocks[oldBlockIdx + 1].y) {
                        // Moved past an element
                        mainProgram.heightSortedBlocks.splice(oldBlockIdx,
                            2, mainProgram.heightSortedBlocks[oldBlockIdx + 1],
                            curDragElement.blockData);
                        oldBlockIdx++;
                        dump("New index is " + oldBlockIdx + "\n");
                    }
                }
            }
            // TODO(rqou): Do snapping logic.
            break;
        default:
            break;
    }

    evt.preventDefault();
    evt.stopPropagation();
}

blockDnD.init = function(document_) {
    document = document_;
    // Main toplevel SVG object
    mainSvg = document.getElementById('blocks-main');

    // Attach the main mouse event handlers
    mainSvg.addEventListener('mousedown', dragMouseDown);
    mainSvg.addEventListener('mouseup', dragMouseUp);
    mainSvg.addEventListener('mousemove', dragMouseMove);
};

blockDnD.setProgram = function(program) {
    mainProgram = program;
};
