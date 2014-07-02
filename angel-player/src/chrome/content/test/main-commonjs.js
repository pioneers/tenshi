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
const JASMINE_STUB_COMMONJS = 'tenshi/test/jasminestub-commonjs';

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
    // Return a version of require() that loads into an isolated environment.
    return jetpackLoader.Require(testLoader, testRequirer);
}

function reportException(e, first_line) {
    console.log(first_line);
    console.log(e.name + ": " + e.message);
    console.log("Stack trace: " + e.stack);
}

// Run simple tests
// These tests are individually loaded into a CommonJS environment and are
// expected to export a function called run that takes no arguments and
// returns a boolean true or false indicating if the test succeeded or not.
// These tests do not have access to the DOM.
function runSimpleTests(window, testDir) {
    let tests = find_recursively(testDir, 'simpletests/');

    let testsAllPassed = true;

    for (let testFileName of tests) {
        let testFilePath = TEST_DIR_COMMONJS + testFileName;
        console.log("Running unit test " + testFileName);
        let testModule;
        try {
            let testEnvRequire = create_new_test_env(window);
            testModule = testEnvRequire(testFilePath);
        } catch(e) {
            testsAllPassed = false;
            reportException(e, "Test " + testFileName + " failed to load!");
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
            reportException(e, "Test " + testFileName + " threw an exception!");
        }
    }

    return testsAllPassed;
}

// Run Jasmine tests
// A new empty CommonJS environment is created and a Jasmine boot.js-like
// stub is loaded into it. This stub preps the newly-created sandbox and
// then loads the actual test file itself. Jasmine will automagically
// run the appropriate tests in the test file. These tests do not have
// access to the DOM.
function runJasmineNoDomTests(window, testDir) {
    let tests = find_recursively(testDir, 'jasmine-no-dom/');

    let testsAllPassed = true;

    for (let testFileName of tests) {
        let testFilePath = TEST_DIR_CHROME + '/' + testFileName;
        console.log("Running unit test " + testFileName);
        let testModule;
        try {
            let testEnvRequire = create_new_test_env(window);
            testModule = testEnvRequire(JASMINE_STUB_COMMONJS);
        } catch(e) {
            testsAllPassed = false;
            reportException(e, "Test " + testFileName + " failed to load!");
        }
        
        try {
            let ret = testModule.run(testFilePath);
            if (!ret) {
                testsAllPassed = false;
                console.log("Test " + testFileName + " failed!");
            } else {
                console.log("OK");
            }
        } catch(e) {
            testsAllPassed = false;
            reportException(e, "Test " + testFileName + " threw an exception!");
        }
    }

    return testsAllPassed;
}

// Run HTML tests
// Each of these tests is loaded into a <browser> element with chrome
// privileges. Jasmine or other frameworks can be added into the page in a
// "normal" way. This is intended for much heavier tests that actually
// require the DOM. To report status, the page needs to raise a TenshiTest
// event, which will be caught by this code. The details field should contain
// a true/false indicating if tests passed or not.
// Get access to the main chrome window
function runHtmlTests(window, testDir, existingTestsAllPassed) {
    let testsAllPassed = existingTestsAllPassed;

    var wm = Cc["@mozilla.org/appshell/window-mediator;1"].
        getService(Ci.nsIWindowMediator);
    var mainWindow = wm.getMostRecentWindow(null);
    var testBrowser = mainWindow.document.getElementById('testBrowser');

    let tests = find_recursively(testDir, 'html-tests/');
    let current_test = 0;

    testBrowser.addEventListener('TenshiTest', function(e) {
        let testPassed = e.detail;
        let testFileName = tests[current_test];

        if (!testPassed) {
            testsAllPassed = false;
            console.log("Test " + testFileName + " failed!");
        } else {
            console.log("OK");
        }

        // Move on to next test or quit
        current_test++;
        if (current_test >= tests.length) {
            // Done
            if (testsAllPassed) {
                console.log("All tests passed!");
            } else {
                console.log("SOME TESTS DID NOT PASS!");
            }

            // Quit
            appStartup.quit(appStartup.eForceQuit);
        } else {
            testFileName = tests[current_test];
            let testFilePath = TEST_DIR_CHROME + '/' + testFileName;
            console.log("Running unit test " + testFileName);
            testBrowser.loadURI(testFilePath);
        }
    });

    // Load the first test
    if (tests.length > 0) {
        let testFileName = tests[0];
        let testFilePath = TEST_DIR_CHROME + '/' + testFileName;
        console.log("Running unit test " + testFileName);
        testBrowser.loadURI(testFilePath);
    }
}

exports.init = function(window) {
    let testDir = url.toFilename(TEST_DIR_CHROME);

    let testsAllPassed = true;

    testsAllPassed = testsAllPassed && runSimpleTests(window, testDir);
    testsAllPassed = testsAllPassed && runJasmineNoDomTests(window, testDir);

    // This must be run last! As far as I can tell, JS in another page cannot
    // run until this JS returns. What we do then is that we load the new page
    // and expect it to raise an event back to us. In the event handler we
    // advance to the next page until there are no more pages. We then exit the
    // process.
    runHtmlTests(window, testDir, testsAllPassed);
};
