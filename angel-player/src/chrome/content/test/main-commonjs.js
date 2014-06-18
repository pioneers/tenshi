const url = require ('jetpack/sdk/url');
const file = require('jetpack/sdk/io/file');
const { Cu, Cc, Ci } = require('chrome');
const console =
    Cu.import("resource://gre/modules/devtools/Console.jsm").console;
const appStartup =
    Cc["@mozilla.org/toolkit/app-startup;1"].getService(Ci.nsIAppStartup);

const TEST_DIR = 'chrome://angel-player/content/test/tests';

exports.onLoad = function(window) {
    let testDir = url.toFilename(TEST_DIR);
    let tests = file.list(testDir);

    let testsAllPassed = true;

    for (let testFileName of tests) {
        let testFilePath = 'tenshi/test/tests/' + testFileName;
        console.log("Running unit test " + testFileName);
        let testModule;
        try {
            testModule = require(testFilePath);
        } catch(e) {
            testsAllPassed = false;
            console.log("Test " + testFileName + " failed to load!");
        }
        
        let ret;
        try {
            ret = testModule.run(window);
            if (!ret) {
                testsAllPassed = false;
                console.log("Test " + testFileName + " failed!");
            } else {
                console.log("OK");
            }
        } catch(e) {
            testsAllPassed = false;
            console.log("Test " + testFileName + " threw an exception!");
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
