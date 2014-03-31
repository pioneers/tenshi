var window,
    ace;

var editor,
    initialized = false;

exports.get_text = function() {
  return editor.getValue();
};

exports.initialized = function(){
    return initialized;
};

exports.init = function(_window) {
  window = _window;
  ace = window.ace;

  editor = ace.edit("editor");
  initialized = true;
};
