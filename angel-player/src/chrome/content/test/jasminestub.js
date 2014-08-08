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

// This file boots up Jasmine. It serves the same function as Jasmine's boot.js,
// but it has been modified to work in the CommonJS environment rather than a
// web page. Note that boot.js is intended to be modified by the end-user of
// Jasmine. This file loads up Jasmine and injects the appropriate Jasmine
// functions used for test specs into the current global scope (which is still
// a sandbox scope). The test infrastructure will then call the run() function
// here, which will invoke Jasmine's env.execute() function and return
// true/false.

const jasmine_module = require('jasmine');
const jasmine_console_module = require('jasmine-console');
const jasmine = jasmine_module.core(jasmine_module);
const jasmine_console =
    jasmine_console_module.console(jasmine_console_module, jasmine);

const { Cc, Ci } = require('chrome');
const { loadSubScript } = Cc['@mozilla.org/moz/jssubscript-loader;1'].
                     getService(Ci.mozIJSSubScriptLoader);

var env = jasmine.getEnv();

// Globals to be injected (used in test specs)
var jasmineInterface = {
    describe: function(description, specDefinitions) {
        return env.describe(description, specDefinitions);
    },

    xdescribe: function(description, specDefinitions) {
        return env.xdescribe(description, specDefinitions);
    },

    it: function(desc, func) {
        return env.it(desc, func);
    },

    xit: function(desc, func) {
        return env.xit(desc, func);
    },

    beforeEach: function(beforeEachFunction) {
        return env.beforeEach(beforeEachFunction);
    },

    afterEach: function(afterEachFunction) {
        return env.afterEach(afterEachFunction);
    },

    expect: function(actual) {
        return env.expect(actual);
    },

    pending: function() {
        return env.pending();
    },

    spyOn: function(obj, methodName) {
        return env.spyOn(obj, methodName);
    },

    jsApiReporter: new jasmine.JsApiReporter({
        timer: new jasmine.Timer()
    }),
};

function extend(destination, source) {
    for (var property in source) destination[property] = source[property];
    return destination;
}
extend(this, jasmineInterface);

/**
 * Expose the interface for adding custom equality testers.
 */
jasmine.addCustomEqualityTester = function(tester) {
    env.addCustomEqualityTester(tester);
};

/**
 * Expose the interface for adding custom expectation matchers
 */
jasmine.addMatchers = function(matchers) {
    return env.addMatchers(matchers);
};

/**
 * Expose the mock interface for the JavaScript timeout functions
 */
jasmine.clock = function() {
    return env.clock;
};

env.catchExceptions(false);

env.addReporter(jasmineInterface.jsApiReporter);

var testsAllPassed = true;

var consoleReporter = new jasmine.ConsoleReporter({
    timer: new jasmine.Timer(),
    print: function() {
        console.log.apply(console, arguments);
    },
    onComplete: function(success) {
        if (!success) {
            testsAllPassed = false;
        }
    }
});
env.addReporter(consoleReporter);

// Actually execute the tests. This stub will load testFilePath as a Jasmine
// spec by loading it into the current global environment.
exports.run = function(testFilePath) {
    // Load the spec
    loadSubScript(testFilePath);

    // Run the tests
    env.execute();

    // Return whether things worked or not
    return testsAllPassed;
};
