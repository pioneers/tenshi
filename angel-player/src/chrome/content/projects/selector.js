var storage = window.localStorage;

var serializers = {},
    deserializers = {};

function connect_storage(key, serialize, deserialize) {
  serializers[key] = serialize;
  deserializers[key] = deserialize;
  deserialize(window.localStorage[key]);
}

function deserialize_all() {
  for (var key in deserializers) {
    if (deserializers.hasOwnProperty(key)) {
      deserializers[key](window.localStorage[key]);
    }
  }
  update_screen();
}

function serialize_all() {
  for (var key in serializers) {
    if (serializers.hasOwnProperty(key)) {
      window.localStorage[key] = serializers[key]();
    }
  }
  window.localStorage["date"] = Date();
  update_screen();
}

function reset_all() {
  for (var key in deserializers) {
    if (deserializers.hasOwnProperty(key)) {
      deserializers[key](undefined);
    }
  }
  update_screen("Reset to defaults. ")
}

function clear_save() {
  window.localStorage.clear();
  update_screen();
}

function update_screen() {
  var prefix = "";
  if (arguments.length > 0) {
    prefix = arguments[0];
  }
  var date = window.localStorage["date"];
  if (date === undefined) {
    $("#text").text(prefix + "No saves available");
  } else {
    $("#text").text(prefix + "Last saved: " + window.localStorage["date"]);
  }
}

var exports = {};
exports.connect_storage = connect_storage;
exports.deserialize_all = deserialize_all;
exports.serialize_all = serialize_all;
exports.reset_all = reset_all;
exports.clear_save = clear_save;
