/* jshint globalstrict: true */
"use strict";

const file = require('jetpack/sdk/io/file');
const js_yaml = require('js-yaml/lib/js-yaml');

function saveApplication(filename) {
    /* jshint validthis: true */

    let appstr = js_yaml.safeDump(this, {skipInvalid: true});
    let outF = file.open(filename, 'w');
    // TODO(rqou): Async?
    outF.write(appstr);
    outF.close();
}

function setExports(obj) {
    obj.save = saveApplication;
}

// Creates an empty robot application
exports.CreateEmptyRobotApplication = function() {
    let ret = {};

    ret.name = "New Robot Application";

    setExports(ret);
    return ret;
};

// Load an application from a file
exports.LoadRobotApplication = function(filename) {
    let appstr = file.read(filename, 'r');
    let ret = js_yaml.safeLoad(appstr);

    setExports(ret);
    return ret;
};
