/* jshint globalstrict: true */
/* global jetpackLoader */
"use strict";

const url = require ('jetpack/sdk/url');
const fs = require('jetpack/sdk/io/fs');
const { Cu, Cc, Ci } = require('chrome');
const console =
    Cu.import("resource://gre/modules/devtools/Console.jsm").console;
const appStartup =
    Cc["@mozilla.org/toolkit/app-startup;1"].getService(Ci.nsIAppStartup);

const TEST_DIR_CHROME = 'chrome://angel-player/content/test';
const TEST_DIR_COMMONJS = 'tenshi/test/';

// files = find_recursively(root, [relative])
// Returns a list of files in <root>/<relative>, or just <root> if relative is
// not passed in. If you pass in relative, relative must end in '/'
// TODO(rqou): This will be moved into a module in common.
function find_recursively(root, relative) {
    let files = [];

    // Walk down the directory tree, and include all files in subdirectories.
    // Will get stuck on directory loops.
    function addFiles (root, relative) {
      // Invariant: (root + '/' + relative) ends with '/'
      for (let f of fs.readdirSync(root + '/' + relative)) {
        let path = root + '/' + relative + f;
        let stat = fs.statSync(path);
        if (stat.isDirectory()) {
          addFiles(root, relative + f + '/');
        } else {
          files.push(relative + f);
        }
      }
    }

    addFiles(root, relative || '');

    return files;
}

// TODO(rqou): Figure out how to move this setting up of loaders into a module.
// This is tricky because (almost by definition) in many cases this is outside
// of the module environment.
function create_new_test_env(window) {
    // Create a brand new loader object to load new tests into. This
    // makes sure that tests cannot affect and contaminate each other.
    let testLoader = jetpackLoader.Loader({
        globals: {
            // These are needed both to make Angelic work and to make Jasmine.js
            // work. They should not be actually bound to the window object in
            // any way. TODO(rqou): Check that this is indeed the case.
            setTimeout: window.setTimeout,
            clearTimeout: window.clearTimeout,
            setInterval: window.setInterval,
            clearInterval: window.clearInterval,
            console: console,
        },
        paths: {
            // In order to make node.js and other modules happy, the
            // "root" of the import paths is the vendor js directory.
            // To get the Mozilla Jetpack SDK stuff, use jetpack/
            "": "chrome://angel-player/content/vendor-js/",
            "jetpack": "resource://gre/modules/commonjs",
            "tenshi": "chrome://angel-player/content",
        },
    });
    let testRequirer = jetpackLoader.Module("TEST_MAIN", "about:blank");
    return jetpackLoader.Require(testLoader, testRequirer);
}

exports.init = function(window) {
    // -------------------------------------------------------------------------
    // Run simple tests
    // These tests are individually loaded into a CommonJS environment and are
    // expected to export a function called run that takes no arguments and
    // returns a boolean true or false indicating if the test succeeded or not.
    // These tests do not have access to the DOM.
    let testDir = url.toFilename(TEST_DIR_CHROME);
    let tests = find_recursively(testDir, 'simpletests/');

    let testsAllPassed = true;

    for (let testFileName of tests) {
        let testFilePath = TEST_DIR_COMMONJS + testFileName;
        console.log("Running unit test " + testFileName);
        let testModule;
        try {
            let testRequire = create_new_test_env(window);
            testModule = testRequire(testFilePath);
        } catch(e) {
            testsAllPassed = false;
            console.log("Test " + testFileName + " failed to load!");
            console.log(e.name + ": " + e.message);
            console.log("Stack trace: " + e.stack);
        }
        
        try {
            let ret = testModule.run();
            if (!ret) {
                testsAllPassed = false;
                console.log("Test " + testFileName + " failed!");
            } else {
                console.log("OK");
            }
        } catch(e) {
            testsAllPassed = false;
            console.log("Test " + testFileName + " threw an exception!");
            console.log(e.name + ": " + e.message);
            console.log("Stack trace: " + e.stack);
        }
    }

    if (testsAllPassed) {
        console.log("All tests passed!");
    } else {
        console.log("SOME TESTS DID NOT PASS!");
    }

    // Quit
    appStartup.quit(appStartup.eForceQuit);
};
