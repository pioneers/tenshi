var window,
    $,
    ace;

var angelic = require("tenshi/angelic/robot"),
    controls = require("tenshi/controls/main");

var editor;

function gen_vm() {
    var vm = angelic.make();

    function set_motor() {}
    function get_sensor(port) {
        return 0;
    }

    vm.add_library ( 'core', [
        vm.make_exfn ( 0, 'print', print_line),
        vm.make_exfn ( 1, 'set_motor', set_motor ),
        vm.make_exfn ( 2, 'get_sensor', get_sensor ),
    ] );

    print_line("====== RESTART ======");
    vm.source = "Console";
    return vm;
}

function onResume() {
    controls.set_vm_generator(gen_vm);
}

function print_line(text) {
    editor.setValue(editor.getValue() + "\n" + text);
    // HACK: for some reason, setting the value selects text
    editor.getSelection().clearSelection();
}

exports.report_error = function(what) {
    print_line("ERROR: " + what);
};

var initialized = false;
exports.initialized = function() {
    return initialized;
};

exports.init = function(_window) {
    window = _window;
    $ = window.$;
    ace = window.ace;

    editor = ace.edit("text");
    editor.setTheme("ace/theme/terminal");
    editor.getSession().setMode("ace/mode/text");
    editor.setReadOnly(true);

    editor.setHighlightActiveLine(false);
    editor.setDisplayIndentGuides(false);
    editor.getSession().setUseWrapMode(true);
    editor.renderer.setShowGutter(false);
    editor.renderer.setPrintMarginColumn(false);

    $("#clear").click(function(){
        $("#text").text("");
    });

    window.onResume = onResume;
    onResume();

    initialized = true;
};
