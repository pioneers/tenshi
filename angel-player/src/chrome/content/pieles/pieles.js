/* jshint globalstrict: true */
"use strict";

let window;
let connectedGamepads = {};
// Maps from values to send to robot to the gamepad id and axis/button
let pielesValueMap = {};
let controllersValid = false;
let updateSetIntervalId;
let attachedRadios = {};

const global_state = require('tenshi/common/global_state');

// 10 Hz. To be tuned later
const PIELES_UPDATE_RATE = 1.0/10.0;

// Checks whether we have all the controllers needed for PiELES to work
// according to the configuration
function validateControllers() {
    let pielesConfig = global_state.get('robot_application').PiELES_config;
    if (!pielesConfig) {
        // No configuration is currently treated as never ready.
        return false;
    }

    // Mapping from controller IDs (which identify the type) to how many we
    // need.
    let required_controllers = {};

    for (let cmdPacketField of pielesConfig) {
        let controllerType = cmdPacketField.controller_type;
        let controllerIdx = cmdPacketField.controller_index;
        if (required_controllers[controllerType] === undefined ||
            required_controllers[controllerType] < controllerIdx + 1) {
            required_controllers[controllerType] = controllerIdx + 1;
        }
    }

    // Now check if we have enough of each controller type
    for (let controller in connectedGamepads) {
        if (required_controllers[connectedGamepads[controller].id]) {
            required_controllers[connectedGamepads[controller].id]--;
        }
    }
    for (let i in required_controllers) {
        if (required_controllers[i] > 0) {
            return false;
        }
    }

    return true;
}

function updatePiELESMap() {
    let pielesConfig = global_state.get('robot_application').PiELES_config;

    for (let cmdPacketField of pielesConfig) {
        let { controller_type, controller_index, name, type } = cmdPacketField;
        if (!(/^PiELESAnalogVals/.test(name)) &&
            !(/^PiELESDigitalVals/.test(name))) {
            // TODO(rqou): Remove this restriction
            throw "Unsupported PiELES field name!";
        }

        // TODO(rqou): This seems really inefficient
        let connectedControllerIndices = Object.keys(connectedGamepads);
        connectedControllerIndices.sort();
        let foundIdx;
        for (let i = 0; i < connectedControllerIndices.length; i++) {
            let gamepad = connectedGamepads[connectedControllerIndices[i]];
            if (gamepad.id === controller_type) {
                if (controller_index === 0) {
                    foundIdx = connectedControllerIndices[i];
                    break;
                }
                controller_index--;
            }
        }

        if (foundIdx === undefined) {
            throw "PiELES somehow couldn't find controller!";
        }

        if (type === "analog") {
            let { axis } = cmdPacketField;
            pielesValueMap[name] = {idx: foundIdx, type: type, axis: axis};
        } else if (type === "digital") {
            let { button } = cmdPacketField;
            pielesValueMap[name] = {idx: foundIdx, type: type, button: button};
        } else {
            throw "Unsupported controller input type " + type;
        }
    }
}

function gamepadAttach(e) {
    let gamepad = e.gamepad;
    connectedGamepads[gamepad.index] = gamepad;

    controllersValid = validateControllers();
    if (controllersValid) {
        updatePiELESMap();
    }
}

function gamepadDetach(e) {
    let gamepad = e.gamepad;
    delete connectedGamepads[gamepad.index];

    controllersValid = validateControllers();
    if (controllersValid) {
        updatePiELESMap();
    }
}

// Does actual sending of packets to robot
// TODO(rqou): Reinvent this protocol
function sendPiELESData(e) {
    let PiELESAnalogVals = [];
    let PiELESDigitalVals = [];

    for (let i = 0; i < 7; i++) {
        let mappedController = pielesValueMap["PiELESAnalogVals" + i];
        // 127 = center, idle
        let value = 127;
        if (mappedController) {
            if (mappedController.type === "analog") {
                value = connectedGamepads[mappedController.idx]
                    .axes[mappedController.axis];

                // Apply scaling
                value = value * 128 + 127;
                if (value < 0) value = 0;
                value = value|0;
            } else if (mappedController.type === "digital") {
                value = connectedGamepads[mappedController.idx]
                    .buttons[mappedController.button].pressed;

                if (value) {
                    value = 255;
                } else {
                    value = 0;
                }
            }
        }
        PiELESAnalogVals[i] = value;
    }

    for (let i = 0; i < 8; i++) {
        let mappedController = pielesValueMap["PiELESDigitalVals" + i];
        let value = false;
        if (mappedController) {
            if (mappedController.type === "analog") {
                value = connectedGamepads[mappedController.idx]
                    .axes[mappedController.axis];

                // TODO(rqou): Customizable threshold?
                value = value > 0;
            } else if (mappedController.type === "digital") {
                value = connectedGamepads[mappedController.idx]
                    .buttons[mappedController.button].pressed;
            }
        }
        PiELESDigitalVals[i] = value;
    }

    // Now we send a packet to the robot!
    for (let radio in attachedRadios) {
        attachedRadios[radio].send({
            'PiELESDigitalVals': PiELESDigitalVals,
            'PiELESAnalogVals': PiELESAnalogVals,
        });
    }
}

// Add a "radio" to PiELES
// Note that it is likely that radio is not really a radio.
// It should have a method in it called send, which accepts a key-value mapping 
// of values.
exports.attachRadio = function(name, radio) {
    attachedRadios[name] = radio;
};

exports.detachRadio = function(name) {
    delete attachedRadios[name];
};

exports.attachToPage = function(_window) {
    // TODO(rqou): Some means to send feedback to update page display when
    // things happen (joystick connect, data RX, etc.)

    window = _window;
    window.addEventListener('gamepadconnected', gamepadAttach);
    window.addEventListener('gamepaddisconnected', gamepadDetach);
    updateSetIntervalId =
        window.setInterval(sendPiELESData, PIELES_UPDATE_RATE);
};

exports.detachFromPage = function() {
    window.removeEventListener('gamepadconnected', gamepadAttach);
    window.removeEventListener('gamepaddisconnected', gamepadDetach);
    window.clearInterval(updateSetIntervalId);
};
