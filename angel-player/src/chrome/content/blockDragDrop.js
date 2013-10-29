var EXPORTED_SYMBOLS = ['blockDnD'];

var blockDnD = {};

var MOUSE_MODE_NONE = 0;
var MOUSE_MODE_PANNING = 1;

var document = null;
var mainSvg = null;
var mainProgram = null;
var currentMouseMode = 0;

// Current SVG panning offset.
var svgPanX = 0;
var svgPanY = 0;

// These coordinates are used to track where we have started the drag, relative
// to the element being dragged. This is used for both individual elements and
// the main canvas.
var curDragX = 0;
var curDragY = 0;

function dragMouseDown(evt) {
    // dump(evt);
    if (!currentMouseMode) {
        // Don't handle a new action if we're already doing one.

        switch (evt.button) {
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
