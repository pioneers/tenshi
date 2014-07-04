let window = require('tenshi/common/window')();
let {ace} = window;

var editor,
    initialized = false;

let lastEditorChangeTimeout = -1;

const global_state = require('tenshi/common/global_state');
const robot_application = require('tenshi/common/robot_application');

function saveEditorText() {
    let robotApp = global_state.get('robot_application');
    robotApp.text_code = editor.getValue();
}

function onEditorChanged(e) {
    // TODO(rqou): Is this inefficient?
    if (lastEditorChangeTimeout != -1) {
        window.clearTimeout(lastEditorChangeTimeout);
    }

    // After no typing for 0.5 second, save the text.
    lastEditorChangeTimeout = window.setTimeout(saveEditorText, 500);
}

exports.get_text = function() {
  return editor.getValue();
};

exports.initialized = function(){
    return initialized;
};

exports.init = function() {
  editor = ace.edit("editor");

  let robotApp = global_state.get('robot_application');
  editor.setValue(robotApp.text_code);
  // HACK: for some reason, setting the value selects text
  editor.getSelection().clearSelection();

  editor.getSession().on('change', onEditorChanged);
  // TODO(rqou): JQuery? Is it loaded in this page?
  window.addEventListener('unload', saveEditorText);

  initialized = true;
};
