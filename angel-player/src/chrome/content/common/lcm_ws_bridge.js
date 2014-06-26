// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

/*
  LCM WebSocket Bridge: javascript client library

  This library imitates the LCM APIs, and fulfills requests by forwarding them
  to a websocket server.

  PUBLIC API:

    new LCM(websocket_url)
    on_ready(callback)
    subscribe(channel, msg_type, callback)
    unsubscribe(subscription_id)
    publish(channel, msg)

  EXAMPLE CODE:

    var l = new LCM("ws://localhost:8000");

    l.on_ready(function() {
      var sub = l.subscribe("sprocket/health", "health", function(msg) {
        alert(msg.header.seq);
      });

      window.setTimeout(function() {
        l.unsubscribe(sub);
      }, 1000);

      l.publish("/forest/cmd", {
        __type__: "forest_cmd",
        header: {
          __type__: "header",
          seq: 5,
          time: 0
        },
        lights: [[true, true, false],
                 [true, true, false],
                 [true, true, false],
                 [true, true, false],
                 [true, true, false],
                 [true, true, false],
                 [true, true, false],
                 [true, true, false]],
        servos: [0, 0, 0, 0, 0, 0, 0, 0]
      });
    });
*/


function LCM (ws_uri) {
  // LCM over WebSockets main class
  var self = this;
  this.ws = new WebSocket(ws_uri);
  this.ws_active = false;
  this.ws.onmessage = function(evt) {
    self.delegate(JSON.parse(evt.data));
  };
  this.callbacks = {}; // indexed by subscription id
}

LCM.prototype.on_ready = function(cb) {
  // Call the callback once this LCM instance is ready to receive commands
  var self = this;
  this.ws.onopen = function () { cb(self); };
};

LCM.prototype.ws_send = function(type, data) {
  // Internal convenience method for sending data over the websocket
  this.ws.send(JSON.stringify({type: type, data:data}));
};

LCM.prototype.delegate = function(request) {
  // Internal method that delegates data received to the appropriate handler
  var callback;
  switch(request.type) {
  case "packet":
    callback = this.callbacks[request.data.subscription_id];
    if (callback !== undefined) {
      callback(request.data.msg);
    }
    break;
  default:
    throw "Invalid request!";
  }
};

LCM.prototype.generate_uuid = function() {
  // Internal method to generate unique subscription IDs
  var d = new Date().getTime();
  var uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    var r = (d + Math.random()*16)%16 | 0;
    d = Math.floor(d/16);
    return (c=='x' ? r : (r&0x7|0x8)).toString(16);
  });
  return uuid;
};

LCM.prototype.subscribe = function(channel, msg_type, callback) {
  // Subscribe to an LCM channel with a callback
  // Unlike the core LCM APIs, this requires a message type, and the
  // callback receives an already-decoded message as JSON instead of
  // an encoded string
  //
  // Invalid requests are silently ignored (there is no error callback)

  var subscription_id = this.generate_uuid();
  this.callbacks[subscription_id] = callback;

  this.ws_send("subscribe", {channel: channel,
                             msg_type: msg_type,
                             subscription_id: subscription_id});
  return subscription_id;
};

LCM.prototype.unsubscribe = function(subscription_id) {
  // Unsubscribe from an LCM channel, using a subscription id
  //
  // Invalid requests are silently ignored (there is no error callback)

  this.ws_send("unsubscribe", {subscription_id: subscription_id});
  delete this.callbacks[subscription_id];
};

LCM.prototype.publish = function(channel, data) {
  // Publish a message to an LCM channel
  // Unlike the core LCM APIs, the data is an arbitrary object, not an
  // instance of something special. However, it and any nested types must
  // define a __type__ relative to the TYPES_ROOT of the websocket server
  //
  // Invalid requests are silently ignored (there is no error callback)
  this.ws_send("publish", {channel: channel,
                           data: data});
};

// Exports for running in a CommonJS environment
if (typeof require !== "undefined") {
  exports.LCM = LCM;
}
