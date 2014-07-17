/*
  Supervisor: interface to field control

  PUBLIC API:

    new Supervisor([station_number, [websocket_url]])
      station_number: one of [0, 1, 2, 3]. Defaults to 0
      websocket_url: defaults to "ws://localhost:8000"
    connect()
      automatically called when the Supervisor is created
    disconnect()

  TODO: figure out what public API to expose to the rest of the app
*/

function proxy (fn, context) {
  // Poor man's version of jQuery.proxy
  // Binds the "this" inside fn to the specified context
  return function () {
    return fn.apply(context, arguments);
  };
}

function Supervisor (station_number, ws_uri) {
  this.station_number = station_number;
  this.ws_uri = ws_uri;
  if (ws_uri === undefined) {
    this.ws_uri = "ws://localhost:8000";
  }
  if (station_number === undefined) {
    this.station_number = 0;
  }
  this.lcm = null;
  this.cmd_sub = "";
  this.config_sub = "";
  this.disconnect();
  this.connect();
}

Supervisor.prototype.connect = function() {
  var self = this;
  this.lcm = new LCM("ws://localhost:8000");
  this.lcm.on_ready(function() {
    self.cmd_sub = self.lcm.subscribe(
      "piemos" + self.station_number + "/cmd",
      "piemos_cmd",
      proxy(self._handle_cmd, self));
    self.cmd_sub = self.lcm.subscribe(
      "PiEMOS" + self.station_number + "/Config",
      "ConfigData",
      proxy(self._handle_config, self));
  });
};

Supervisor.prototype.disconnect = function() {
  if (this.lcm !== null) {
    this.lcm.unsubscribe(this.cmd_sub);
    this.lcm.unsubscribe(this.config_sub);
    this.cmd_sub = "";
    this.config_sub = "";
    this.lcm = null;
  }
};

Supervisor.prototype._handle_cmd = function(msg) {
  // Message format:
  //   {
  //     enabled: bool,
  //     is_blue: bool,
  //     auton: bool,
  //     game_time: int (seconds),
  //   }

  console.log("Got state from field control");
};

Supervisor.prototype._handle_config = function(msg) {
  // Message format:
  //   {
  //     IsBlue: bool,
  //     TeamName: string,
  //     TeamNumber: int
  //     ...
  //   }

  // Field control still passes PiEMOS configs
  // Here is how to extract the radio address from the config:
  this.radio_address = JSON.parse(msg.ConfigFile).radioAddress;
  console.log("Got config from field control: " + this.radio_address);
};

// Exports for running in a CommonJS environment
if (typeof require !== "undefined") {
  exports.Supervisor = Supervisor;
}
