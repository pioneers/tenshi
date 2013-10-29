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
                while (targetElem.getAttributeNS(null, 'class') !== 'codeBlock'
                    && targetElem !== mainSvg) {
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
    // dump(evt);
    switch (currentMouseMode) {
        case MOUSE_MODE_PANNING:
            // Panning the whole canvas
            svgPanX = evt.clientX - curDragX;
            svgPanY = evt.clientY - curDragY;
            var transform = mainSvg.createSVGTransform();
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

            var transform = mainSvg.createSVGTransform();
            transform.setTranslate(newX, newY);
            curDragElement.transform.baseVal.initialize(transform);

            // TODO(rqou): Update all the relevant state for element position.
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
