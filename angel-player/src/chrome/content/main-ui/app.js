const studentconsole = require('tenshi/console/console');
const texteditor = require('tenshi/texteditor/editor');
const ioports = require('tenshi/welcome/ioports');
const radio = require('tenshi/common/radio');
const global_state = require('tenshi/common/global_state');

exports.init = function(_window) {
  _window.$(function() {
    var angular = _window.angular;
    var app = angular.module("tenshi", ['ui.ace']);

    studentconsole.consoleController(app);
    texteditor.texteditorController(app);
    ioports.ioportController(app);

    angular.bootstrap(_window.document, [app.name]);

    // Initialize global state.
    global_state.set('main_radio', new radio.Radio());

    // Following Google's angular style guide (but modified to use CommonJS
    // instead of the goog import system), other files that need to declare an
    // angular dependency on a module should use the following:
    // angular.module("name", [require("tenshi/main-ui/app").module.name])
    //
    // Dependencies should never be declared as strings. This makes the
    // cross-file references more explicit and less prone to errors.

    // TODO(nikita): does setting exports inside init actually work? Or do we need
    // a getter function?
    exports.module = app;
  });
};
