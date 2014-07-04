// This file exposes the window object (corresponding to ui.html) to the
// CommonJS environment, as well as any libraries defined in that window
// object.
//
// Usage:
//   const window = require('tenshi/common/window')();
//   let { document, $, ... } = window;

// Get the "global" window. This is right outside the iframe that has
// ui.html in it
const global_window = require('get_global_window')();

// Make the window inside the iframe available to code
module.exports = function() {
  return global_window.document.getElementById('mainContent').contentWindow;
};