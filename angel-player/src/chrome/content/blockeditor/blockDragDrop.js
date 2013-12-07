// This radius is in em.
var BLOCK_SNAP_RADIUS = 0.5;

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

// This is (currently) used to implement heirarchical blocks.
// TODO(rqou): The behavior of heirarchical blocks isn't clear. Work on this
// later.
function recursivelyReparentNodes(startNode, newParent) {
    startNode.parent = newParent;
    if (startNode.rightPeer) {
        recursivelyReparentNodes(startNode.rightPeer, newParent);
    }
    if (startNode.nextBlock) {
        recursivelyReparentNodes(startNode.nextBlock, newParent);
    }
}

function dragMouseDown(evt) {
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

                    curDragElement = targetElem;
                    // TODO(rqou): Do we need to copypasta this?
                    targetElem.setAttributeNS(null, 'pointer-events', 'none');
                    currentMouseMode = MOUSE_MODE_DRAG_BLOCK;

                    // Remove all of the links to the other blocks

                    // If we are being dragged away, everything "attached to"
                    // this block (things on the right and below) become
                    // detached from any parent we may be attached to.
                    // TODO(rqou): This is almost certainly not the expected
                    // behavior. Among other things, the blocks that get
                    // detached here don't physically move, and putting the
                    // block back probably won't reattach things properly.
                    recursivelyReparentNodes(curDragElement.blockData, null);

                    // Normal left/right/prev/next links
                    if (curDragElement.blockData.leftPeer) {
                        curDragElement.blockData.leftPeer.rightPeer = null;
                        curDragElement.blockData.leftPeer = null;
                    }

                    if (curDragElement.blockData.rightPeer) {
                        curDragElement.blockData.rightPeer.leftPeer = null;
                        curDragElement.blockData.rightPeer = null;
                    }

                    if (curDragElement.blockData.prevBlock) {
                        curDragElement.blockData.prevBlock.nextBlock = null;
                        curDragElement.blockData.prevBlock = null;
                    }

                    if (curDragElement.blockData.nextBlock) {
                        curDragElement.blockData.nextBlock.prevBlock = null;
                        curDragElement.blockData.nextBlock = null;
                    }

                    // If this block has children, abandon them.
                    if (curDragElement.blockData.firstChild) {
                        recursivelyReparentNodes(
                            curDragElement.blockData.firstChild, null);
                        curDragElement.blockData.firstChild = null;
                    }

                    // If we have a parent, and we are the first node, remove
                    // ourselves. Otherwise, we have already been "lost" when
                    // we disconnect the prev/next pointers.
                    if (curDragElement.blockData.parent &&
                            (curDragElement.blockData.parent.firstChild == 
                            curDragElement.blockData)) {
                        curDragElement.blockData.parent.firstChild = null;
                    }
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
    var i;

    switch (evt.button) {
        case 0:
            if (currentMouseMode == MOUSE_MODE_DRAG_BLOCK) {
                // Logic to snap blocks in sequence and horizontally
                var neighboringBlocks = mainProgram.getBlocksInArea(
                    curDragElement.blockData.x - BLOCK_SNAP_RADIUS,
                    curDragElement.blockData.y - BLOCK_SNAP_RADIUS,
                    curDragElement.blockData.w + 2 * BLOCK_SNAP_RADIUS,
                    curDragElement.blockData.h + 2 * BLOCK_SNAP_RADIUS);

                // Exclude the currently-dragged block
                // TODO(rqou): not hacky, more efficient
                for (i = 0; i < neighboringBlocks.length; i++) {
                    if (neighboringBlocks[i] === curDragElement.blockData) {
                        neighboringBlocks.splice(i, 1);
                        i--;
                    }
                }

                // Exclude all things that we don't half-overlap with
                // horizontally or vertically. This is the heuristic we use to
                // decide if things should snap.
                var overlapAmountHoriz = 0;
                var overlapAmountVert = 0;
                var block = null;
                for (i = 0; i < neighboringBlocks.length; i++) {
                    block = neighboringBlocks[i];
                    // Horizontal
                    if (curDragElement.blockData.x < block.x)
                        overlapAmountHoriz = curDragElement.blockData.x +
                            curDragElement.blockData.w - block.x;
                    else
                        overlapAmountHoriz = block.x + block.w -
                            curDragElement.blockData.x;
                    // Vertical
                    if (curDragElement.blockData.y < block.y)
                        overlapAmountVert = curDragElement.blockData.y +
                            curDragElement.blockData.h - block.y;
                    else
                        overlapAmountVert = block.y + block.h -
                            curDragElement.blockData.y;

                    // Find the shortest width/height of the two interacting
                    // blocks
                    var shortestWidth =
                        block.w < curDragElement.blockData.w ?
                        block.w :
                        curDragElement.blockData.w;
                    var shortestHeight =
                        block.h < curDragElement.blockData.h ?
                        block.h :
                        curDragElement.blockData.h;

                    // Finally exclude things that are not half-overlapped in
                    // either axis
                    if (overlapAmountHoriz < 0.5 * shortestWidth &&
                        overlapAmountVert < 0.5 * shortestHeight) {
                        neighboringBlocks.splice(i, 1);
                        i--;
                    }
                }

                var newX = curDragElement.blockData.x;
                var newY = curDragElement.blockData.y;

                // For now, we will snap to the first block found.
                // It will snap horizontally/vertically depending on which axis
                // overlaps more.
                // TODO(rqou): How should this work?
                if (neighboringBlocks.length > 0) {
                    block = neighboringBlocks[0];
                    // Horizontal
                    if (curDragElement.blockData.x < block.x)
                        overlapAmountHoriz = curDragElement.blockData.x +
                            curDragElement.blockData.w - block.x;
                    else
                        overlapAmountHoriz = block.x + block.w -
                            curDragElement.blockData.x;
                    var horizOverlapPercent =
                        overlapAmountHoriz / curDragElement.blockData.w;
                    // Vertical
                    if (curDragElement.blockData.y < block.y)
                        overlapAmountVert = curDragElement.blockData.y +
                            curDragElement.blockData.h - block.y;
                    else
                        overlapAmountVert = block.y + block.h -
                            curDragElement.blockData.y;
                    var vertOverlapPercent =
                        overlapAmountVert / curDragElement.blockData.h;

                    if (horizOverlapPercent > vertOverlapPercent) {
                        // Snap above/below
                        newX = block.x;

                        if (curDragElement.blockData.y < block.y) {
                            // The current thing is being snapped above the
                            // other
                            newY = block.y - curDragElement.blockData.h;

                            curDragElement.blockData.nextBlock = block;
                            block.prevBlock = curDragElement.blockData;
                            curDragElement.blockData.parent = block.parent;
                        }
                        else {
                            // The current thing is being snapped below the
                            // other
                            newY = block.y + block.h;

                            // If the other block is a container, we become a
                            // child.
                            // TODO(rqou): This may not be the best way to do
                            // this. Also, there is currently no way to put
                            // statements after containers.
                            if (block.isContainer) {
                                curDragElement.blockData.parent = block;
                                block.firstChild = curDragElement.blockData;
                            }
                            else {
                                curDragElement.blockData.prevBlock = block;
                                block.nextBlock = curDragElement.blockData;
                                curDragElement.blockData.parent = block.parent;
                            }
                        }
                    }
                    else {
                        // Snap left/right
                        newY = block.y;

                        if (curDragElement.blockData.x < block.x) {
                            // The current thing is being snapped left of the
                            // other
                            newX = block.x - curDragElement.blockData.w;

                            curDragElement.blockData.rightPeer = block;
                            block.leftPeer = curDragElement.blockData;
                            curDragElement.blockData.parent = block.parent;
                        }
                        else {
                            // The current thing is being snapped right of the
                            // other
                            newX = block.x + block.w;

                            curDragElement.blockData.leftPeer = block;
                            block.rightPeer = curDragElement.blockData;
                            curDragElement.blockData.parent = block.parent;
                        }
                    }

                    curDragElement.blockData.moveTo(newX, newY);
                }

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

            curDragElement.blockData.moveTo(newX, newY);

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
                    }
                }
            }
            break;
        default:
            break;
    }

    evt.preventDefault();
    evt.stopPropagation();
}

exports.init = function(document_) {
    document = document_;
    // Main toplevel SVG object
    mainSvg = document.getElementById('blocks-main');

    // Attach the main mouse event handlers
    mainSvg.addEventListener('mousedown', dragMouseDown);
    mainSvg.addEventListener('mouseup', dragMouseUp);
    mainSvg.addEventListener('mousemove', dragMouseMove);
};

exports.setProgram = function(program) {
    mainProgram = program;
};
