/* jshint globalstrict: true */
"use strict";

let window;
let connectedGamepads = {};
// Maps from values to send to robot to the gamepad id and axis/button
let pielesValueMap = {};
let controllersValid = false;
let updateSetIntervalId;
const global_state = require('tenshi/common/global_state');
const robot_application = require('tenshi/common/robot_application');
const typpo_module = require('tenshi/angelic/factory');
const url = require('jetpack/sdk/url');
const buffer = require('jetpack/sdk/io/buffer');
const Int64 = require('Int64.js');

const XBEE_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/xbee_typpo.yaml';
const PIEMOS_FRAMING_YAML_FILE =
    'chrome://angel-player/content/common_defs/legacy_piemos_framing.yaml';

// 10 Hz. To be tuned later
const PIELES_UPDATE_RATE = 1.0/10.0;

// Init Typpo
let typpo = typpo_module.make();
typpo.set_target_type('ARM');
typpo.load_type_file(url.toFilename(XBEE_FRAMING_YAML_FILE), false);
typpo.load_type_file(url.toFilename(PIEMOS_FRAMING_YAML_FILE), false);

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

// TODO(rqou): Move this elsewhere. There's now two copies.
function computeXbeeChecksum(buf, start, len) {
    let sum = 0;

    for (let i = start; i < start + len; i++) {
        sum += buf[i];
    }

    sum = sum & 0xFF;

    return 0xFF - sum;
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

    let digitalBitfield = 0;
    for (let i = 0; i < 8; i++) {
        if (PiELESDigitalVals[i]) {
            digitalBitfield |= (1 << i);
        }
    }

    // Now we send a packet to the robot!

    // TODO(rqou): Massive massive code duplication alert! Clean me up!

    let serportObj = global_state.get('serial_port_object');
    if (!serportObj) {
        throw "Serial port not open!";
    }

    let robotApp = global_state.get('robot_application');
    if (!robotApp.radio_pairing_info) {
        throw "No radio address set!";
    }
    let ROBOT = "0x" + robotApp.radio_pairing_info;

    // Create initial packet
    let initial_packet = typpo.create('pier_incomingdata');
    initial_packet.set_slot('ident',
        typpo.get_const('PIER_INCOMINGDATA_IDENT'));
    // TODO(rqou): Meaningful flags?
    initial_packet.set_slot('fieldtime', 0);
    initial_packet.set_slot('flags', 0);
    initial_packet.set_slot('analog', PiELESAnalogVals);
    initial_packet.set_slot('digital', digitalBitfield);
    // The +1 allows the checksum to be crammed in the end
    let payloadbuf =
        buffer.Buffer(typpo.get_size('pier_incomingdata') + 1);
    payloadbuf.fill(0x00);
    initial_packet.write(payloadbuf);
    // Add XBee framing
    let xbee_tx_frame = typpo.create('xbee_tx64_header');
    xbee_tx_frame.set_slot('xbee_api_type',
        typpo.get_const('XBEE_API_TYPE_TX64'));
    // TODO(rqou): Use frameId?
    xbee_tx_frame.set_slot('frameId', 0);
    xbee_tx_frame.set_slot('xbee_dest_addr', new Int64(ROBOT));
    xbee_tx_frame.set_slot('options', 0);
    xbee_tx_frame.set_slot('data', payloadbuf);
    let xbee_payload = typpo.create('xbee_payload');
    xbee_payload.set_slot('tx64', xbee_tx_frame);
    let initial_packet_xbee = typpo.create('xbee_api_packet');
    initial_packet_xbee.set_slot('xbee_api_magic',
        typpo.get_const('XBEE_MAGIC'));
    initial_packet_xbee.set_slot('length',
        typpo.get_size('pier_incomingdata') +
        typpo.get_size('xbee_tx64_header'));
    initial_packet_xbee.set_slot('payload', xbee_payload);
    // TODO(rqou): Jank jank jank
    let buf = buffer.Buffer(
        typpo.get_size('xbee_api_packet') + payloadbuf.length);
    initial_packet_xbee.write(buf);
    // Note, this is kinda jank. Checksum is last byte. Getting the length
    // is also kinda borked due to the union.
    // TODO(rqou): Don't hardcode 3 here!
    buf[buf.length - 1] = computeXbeeChecksum(buf,
        3,
        buf.length - 1 - (3));

    serportObj.write(buf);
}

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
