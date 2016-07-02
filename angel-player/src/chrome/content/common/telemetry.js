/* jshint globalstrict: true */
"use strict";

const prefs = require('tenshi/common/tenshi_prefs');
// TODO(rqou): This is awful
const { CC } = require('chrome');
const XMLHttpRequest =
  CC("@mozilla.org/xmlextras/xmlhttprequest;1", "nsIXMLHttpRequest");

exports.sendTelemetry = function(stuff) {
  let telemetryUrl = prefs.telemetryUrl();
  if (telemetryUrl) {
    console.log(telemetryUrl);
    // TODO(rqou): This is extremely primitive, we might want to replace this
    // with something smarter later.
    let xhr = new XMLHttpRequest();
    xhr.open("post", telemetryUrl);
    xhr.send(stuff);
    console.log("sent");
  } else {
    console.log("Telemetry skipped");
  }
};
